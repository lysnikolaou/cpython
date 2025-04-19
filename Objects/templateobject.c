/* t-string Template object implementation */

#include "Python.h"
#include "pycore_interpolation.h" // _PyInterpolation_CheckExact()
#include "pycore_template.h"

typedef struct {
    PyObject_HEAD
    PyObject *stringsiter;
    PyObject *interpolationsiter;
    int from_strings;
} templateiterobject;

static PyObject *
templateiter_next(templateiterobject *self)
{
    PyObject *item;
    if (self->from_strings) {
        item = PyIter_Next(self->stringsiter);
        self->from_strings = 0;
        if (PyUnicode_GET_LENGTH(item) == 0) {
            Py_SETREF(item, PyIter_Next(self->interpolationsiter));
            self->from_strings = 1;
        }
    } else {
        item = PyIter_Next(self->interpolationsiter);
        self->from_strings = 1;
    }
    return item;
}

static void
templateiter_dealloc(templateiterobject *self)
{
    PyObject_GC_UnTrack(self);
    Py_CLEAR(self->stringsiter);
    Py_CLEAR(self->interpolationsiter);
    Py_TYPE(self)->tp_free(self);
}

static int
templateiter_traverse(templateiterobject *self, visitproc visit, void *arg)
{
    Py_VISIT(self->stringsiter);
    Py_VISIT(self->interpolationsiter);
    return 0;
}

PyTypeObject _PyTemplateIter_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "string.templatelib.TemplateIter",
    .tp_doc = PyDoc_STR("Template iterator object"),
    .tp_basicsize = sizeof(templateiterobject),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_GC,
    .tp_alloc = PyType_GenericAlloc,
    .tp_dealloc = (destructor) templateiter_dealloc,
    .tp_free = PyObject_GC_Del,
    .tp_traverse = (traverseproc) templateiter_traverse,
    .tp_iter = PyObject_SelfIter,
    .tp_iternext = (iternextfunc) templateiter_next,
};

typedef struct {
    PyObject_HEAD
    PyObject *strings;
    PyObject *interpolations;
} templateobject;

#define templateobject_CAST(op) \
    (assert(_PyTemplate_CheckExact(op)), _Py_CAST(templateobject*, (op)))

static templateobject *
template_from_strings_interpolations(PyTypeObject *type, PyObject *strings, PyObject *interpolations)
{
    templateobject *template = (templateobject *) type->tp_alloc(type, 0);
    if (template == NULL) {
        return NULL;
    }

    template->strings = Py_NewRef(strings);
    template->interpolations = Py_NewRef(interpolations);
    return template;
}

static PyObject *
template_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    if (kwds != NULL) {
        PyErr_SetString(PyExc_TypeError, "Template.__new__ only accepts *args arguments");
        return NULL;
    }

    Py_ssize_t argslen = PyTuple_GET_SIZE(args);
    Py_ssize_t stringslen = 0;
    Py_ssize_t interpolationslen = 0;
    int last_was_str = 0;

    for (Py_ssize_t i = 0; i < argslen; i++) {
        PyObject *item = PyTuple_GET_ITEM(args, i);
        if (PyUnicode_Check(item)) {
            if (!last_was_str) {
                stringslen++;
            }
            last_was_str = 1;
        }
        else if (_PyInterpolation_CheckExact(item)) {
            if (!last_was_str) {
                stringslen++;
            }
            interpolationslen++;
            last_was_str = 0;
        }
        else {
            PyErr_SetString(PyExc_TypeError, "Template.__new__ *args need to be of type 'str' or 'Interpolation'");
            return NULL;
        }
    }
    if (!last_was_str) {
        stringslen++;
    }

    PyObject *strings = PyTuple_New(stringslen);
    if (!strings) {
        return NULL;
    }

    PyObject *interpolations = PyTuple_New(interpolationslen);
    if (!interpolations) {
        Py_DECREF(strings);
        return NULL;
    }

    last_was_str = 0;
    Py_ssize_t stringsidx = 0, interpolationsidx = 0;
    for (Py_ssize_t i = 0; i < argslen; i++) {
        PyObject *item = PyTuple_GET_ITEM(args, i);
        if (PyUnicode_Check(item)) {
            if (last_was_str) {
                PyObject *laststring = PyTuple_GET_ITEM(strings, stringsidx - 1);
                PyObject *concat = PyUnicode_Concat(laststring, item);
                Py_DECREF(laststring);
                if (!concat) {
                    Py_DECREF(strings);
                    Py_DECREF(interpolations);
                    return NULL;
                }
                PyTuple_SET_ITEM(strings, stringsidx - 1, concat);
            }
            else {
                PyTuple_SET_ITEM(strings, stringsidx++, Py_NewRef(item));
            }
            last_was_str = 1;
        }
        else if (_PyInterpolation_CheckExact(item)) {
            if (!last_was_str) {
                PyTuple_SET_ITEM(strings, stringsidx++, &_Py_STR(empty));
            }
            PyTuple_SET_ITEM(interpolations, interpolationsidx++, Py_NewRef(item));
            last_was_str = 0;
        }
    }
    if (!last_was_str) {
        PyTuple_SET_ITEM(strings, stringsidx++, &_Py_STR(empty));
    }

    templateobject *template = template_from_strings_interpolations(type, strings, interpolations);
    Py_DECREF(strings);
    Py_DECREF(interpolations);
    return (PyObject *)template;
}

static void
template_dealloc(PyObject *op)
{
    templateobject *self = templateobject_CAST(op);
    PyObject_GC_UnTrack(self);
    Py_CLEAR(self->strings);
    Py_CLEAR(self->interpolations);
    Py_TYPE(self)->tp_free(self);
}

static int
template_traverse(PyObject *op, visitproc visit, void *arg)
{
    templateobject *self = templateobject_CAST(op);
    Py_VISIT(self->strings);
    Py_VISIT(self->interpolations);
    return 0;
}

static PyObject *
template_repr(PyObject *op)
{
    templateobject *self = templateobject_CAST(op);
    return PyUnicode_FromFormat("%s(strings=%R, interpolations=%R)",
                                _PyType_Name(Py_TYPE(self)),
                                self->strings,
                                self->interpolations);
}

static PyObject *
template_iter(PyObject *op)
{
    templateobject *self = templateobject_CAST(op);
    templateiterobject *iter = PyObject_GC_New(templateiterobject, &_PyTemplateIter_Type);
    if (iter == NULL) {
        return NULL;
    }

    PyObject *stringsiter = PyObject_GetIter(self->strings);
    if (stringsiter == NULL) {
        Py_DECREF(iter);
        return NULL;
    }

    PyObject *interpolationsiter = PyObject_GetIter(self->interpolations);
    if (interpolationsiter == NULL) {
        Py_DECREF(iter);
        Py_DECREF(stringsiter);
        return NULL;
    }

    iter->stringsiter = stringsiter;
    iter->interpolationsiter = interpolationsiter;
    iter->from_strings = 1;
    PyObject_GC_Track(iter);
    return (PyObject *)iter;
}

static PyObject *
template_interpolations_copy(PyObject *interpolations) {
    Py_ssize_t interpolationslen = PyTuple_GET_SIZE(interpolations);
    PyObject *newinterpolations = PyTuple_New(interpolationslen);
    if (!newinterpolations) {
        return NULL;
    }

    for (Py_ssize_t i = 0; i < interpolationslen; i++) {
        PyTuple_SET_ITEM(newinterpolations, i, Py_NewRef(PyTuple_GET_ITEM(interpolations, i)));
    }
    return newinterpolations;
}

static PyObject *
template_interpolations_concat(PyObject *left, PyObject *right) {
    Py_ssize_t leftlen = PyTuple_GET_SIZE(left);
    Py_ssize_t rightlen = PyTuple_GET_SIZE(right);
    Py_ssize_t interpolationslen = leftlen + rightlen;

    PyObject *newinterpolations = PyTuple_New(interpolationslen);
    if (!newinterpolations) {
        return NULL;
    }

    Py_ssize_t index = 0;
    for (Py_ssize_t i = 0; i < leftlen; i++) {
        PyTuple_SET_ITEM(newinterpolations, index++, Py_NewRef(PyTuple_GET_ITEM(left, i)));
    }
    for (Py_ssize_t i = 0; i < rightlen; i++) {
        PyTuple_SET_ITEM(newinterpolations, index++, Py_NewRef(PyTuple_GET_ITEM(right, i)));
    }
    return newinterpolations;
}

static PyObject *
template_strings_append_str(PyObject *strings, PyObject *str)
{
    Py_ssize_t stringslen = PyTuple_GET_SIZE(strings);
    PyObject *string = PyTuple_GET_ITEM(strings, stringslen - 1);
    PyObject *concat = PyUnicode_Concat(string, str);
    if (concat == NULL) {
        return NULL;
    }

    PyObject *newstrings = PyTuple_New(stringslen);
    if (newstrings == NULL) {
        Py_DECREF(concat);
        return NULL;
    }

    for (Py_ssize_t i = 0; i < stringslen - 1; i++) {
        PyTuple_SET_ITEM(newstrings, i, Py_NewRef(PyTuple_GET_ITEM(strings, i)));
    }
    PyTuple_SET_ITEM(newstrings, stringslen - 1, concat);

    return newstrings;
}

static PyObject *
template_strings_prepend_str(PyObject *strings, PyObject *str)
{
    Py_ssize_t stringslen = PyTuple_GET_SIZE(strings);
    PyObject *string = PyTuple_GET_ITEM(strings, 0);
    PyObject *concat = PyUnicode_Concat(str, string);
    if (concat == NULL) {
        return NULL;
    }

    PyObject *newstrings = PyTuple_New(stringslen);
    if (newstrings == NULL) {
        Py_DECREF(concat);
        return NULL;
    }

    PyTuple_SET_ITEM(newstrings, 0, concat);
    for (Py_ssize_t i = 1; i < stringslen; i++) {
        PyTuple_SET_ITEM(newstrings, i, Py_NewRef(PyTuple_GET_ITEM(strings, i)));
    }

    return newstrings;
}

static PyObject *
template_strings_concat(PyObject *left, PyObject *right)
{
    Py_ssize_t left_stringslen = PyTuple_GET_SIZE(left);
    PyObject *left_laststring = PyTuple_GET_ITEM(left, left_stringslen - 1);
    Py_ssize_t right_stringslen = PyTuple_GET_SIZE(right);
    PyObject *right_firststring = PyTuple_GET_ITEM(right, 0);

    PyObject *concat = PyUnicode_Concat(left_laststring, right_firststring);
    if (concat == NULL) {
        return NULL;
    }

    PyObject *newstrings = PyTuple_New(left_stringslen + right_stringslen - 1);
    if (newstrings == NULL) {
        Py_DECREF(concat);
        return NULL;
    }

    Py_ssize_t index = 0;
    for (Py_ssize_t i = 0; i < left_stringslen - 1; i++) {
        PyTuple_SET_ITEM(newstrings, index++, Py_NewRef(PyTuple_GET_ITEM(left, i)));
    }
    PyTuple_SET_ITEM(newstrings, index++, concat);
    for (Py_ssize_t i = 1; i < right_stringslen; i++) {
        PyTuple_SET_ITEM(newstrings, index++, Py_NewRef(PyTuple_GET_ITEM(right, i)));
    }

    return newstrings;
}

static PyObject *
template_concat_templates(templateobject *self, templateobject *other)
{
    PyObject *newstrings = template_strings_concat(self->strings, other->strings);
    if (newstrings == NULL) {
        return NULL;
    }

    PyObject *newinterpolations = template_interpolations_concat(self->interpolations, other->interpolations);
    if (newinterpolations == NULL) {
        Py_DECREF(newstrings);
        return NULL;
    }

    templateobject *newtemplate = template_from_strings_interpolations(Py_TYPE(self), newstrings, newinterpolations);
    Py_DECREF(newstrings);
    Py_DECREF(newinterpolations);
    return (PyObject *) newtemplate;
}

static PyObject *
template_concat_template_str(templateobject *self, PyObject *other)
{
    PyObject *newstrings = template_strings_append_str(self->strings, other);
    if (newstrings == NULL) {
        return NULL;
    }

    PyObject *newinterpolations = template_interpolations_copy(self->interpolations);
    if (newinterpolations == NULL) {
        Py_DECREF(newstrings);
        return NULL;
    }

    templateobject *newtemplate = template_from_strings_interpolations(Py_TYPE(self), newstrings, newinterpolations);
    Py_DECREF(newstrings);
    Py_DECREF(newinterpolations);
    return (PyObject *) newtemplate;
}

static PyObject *
template_concat_str_template(templateobject *self, PyObject *other)
{
    PyObject *newstrings = template_strings_prepend_str(self->strings, other);
    if (newstrings == NULL) {
        return NULL;
    }

    PyObject *newinterpolations = template_interpolations_copy(self->interpolations);
    if (newinterpolations == NULL) {
        Py_DECREF(newstrings);
        return NULL;
    }

    templateobject *newtemplate = template_from_strings_interpolations(Py_TYPE(self), newstrings, newinterpolations);
    Py_DECREF(newstrings);
    Py_DECREF(newinterpolations);
    return (PyObject *) newtemplate;
}

PyObject *
_PyTemplate_Concat(PyObject *self, PyObject *other)
{
    if (_PyTemplate_CheckExact(self) && _PyTemplate_CheckExact(other)) {
        return template_concat_templates((templateobject *) self, (templateobject *) other);
    }
    else if ((_PyTemplate_CheckExact(self)) && PyUnicode_Check(other)) {
        return template_concat_template_str((templateobject *) self, other);
    }
    else if (PyUnicode_Check(self) && (_PyTemplate_CheckExact(other))) {
        return template_concat_str_template((templateobject *) other, self);
    }
    else {
        Py_RETURN_NOTIMPLEMENTED;
    }
}

static PyObject *
template_values_get(PyObject *op, void *Py_UNUSED(data))
{
    templateobject *self = templateobject_CAST(op);
    PyObject *values = PyTuple_New(PyTuple_GET_SIZE(self->interpolations));
    if (values == NULL) {
        return NULL;
    }

    PyObject *interpolationsiter = PyObject_GetIter(self->interpolations);
    if (interpolationsiter == NULL) {
        Py_DECREF(values);
        return NULL;
    }

    PyObject *item;
    Py_ssize_t index = 0;
    while ((item = PyIter_Next(interpolationsiter))) {
        PyTuple_SET_ITEM(values, index++, Py_NewRef(_PyInterpolation_GetValue(item)));
        Py_DECREF(item);
    }

    Py_DECREF(interpolationsiter);
    if (PyErr_Occurred()) {
        Py_DECREF(values);
        return NULL;
    }
    return values;
}

static PyMemberDef template_members[] = {
    {"strings", Py_T_OBJECT_EX, offsetof(templateobject, strings), Py_READONLY, "Strings"},
    {"interpolations", Py_T_OBJECT_EX, offsetof(templateobject, interpolations), Py_READONLY, "Interpolations"},
    {NULL},
};

static PyGetSetDef template_getset[] = {
    {"values", template_values_get, NULL, "Values of interpolations", NULL},
    {NULL},
};

static PySequenceMethods template_as_sequence = {
    .sq_concat = _PyTemplate_Concat,
};

PyTypeObject _PyTemplate_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "string.templatelib.Template",
    .tp_doc = PyDoc_STR("Template object"),
    .tp_basicsize = sizeof(templateobject),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_GC,
    .tp_as_sequence = &template_as_sequence,
    .tp_new = template_new,
    .tp_alloc = PyType_GenericAlloc,
    .tp_dealloc = template_dealloc,
    .tp_free = PyObject_GC_Del,
    .tp_repr = template_repr,
    .tp_members = template_members,
    .tp_getset = template_getset,
    .tp_iter = template_iter,
    .tp_traverse = template_traverse,
};

PyObject *
_PyTemplate_FromValues(PyObject **values, Py_ssize_t oparg)
{
    PyObject *tuple = PyTuple_New(oparg);
    if (!tuple) {
        return NULL;
    }

    for (Py_ssize_t i = 0; i < oparg; i++) {
        PyTuple_SET_ITEM(tuple, i, Py_NewRef(values[i]));
    }

    PyObject *template = (PyObject *) template_new(&_PyTemplate_Type, tuple, NULL);
    Py_DECREF(tuple);
    return template;
}

PyObject *
_PyTemplate_FromList(PyObject *list)
{
    PyObject *tuple = PyList_AsTuple(list);
    if (!tuple) {
        return NULL;
    }

    PyObject *template = (PyObject *) template_new(&_PyTemplate_Type, tuple, NULL);
    Py_DECREF(tuple);
    return template;
}
