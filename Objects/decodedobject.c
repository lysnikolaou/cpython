#include "Python.h"
#include "pycore_initconfig.h"  // _PyStatus_OK
#include "pycore_typeobject.h"  // _PyStaticType_InitBuiltin
#include "pycore_decoded.h"     // PyDecodedObject
#include "pycore_object.h"        // _PyObject_GC_TRACK

#include "clinic/decodedobject.c.h"

#include <stddef.h>

/*[clinic input]
class DecodedConcrete "PyObject *" "&PyDecodedObject"
[clinic start generated code]*/
/*[clinic end generated code: output=da39a3ee5e6b4b0d input=dd19a847469a129c]*/

/*[clinic input]
@classmethod
DecodedConcrete.__new__ as DecodedConcrete_new

    object as x: object = NULL
    encoding: object = NULL
    errors: object = NULL

[clinic start generated code]*/

static PyObject *
DecodedConcrete_new_impl(PyTypeObject *type, PyObject *x, PyObject *encoding,
                         PyObject *errors)
/*[clinic end generated code: output=96e8cda6cd94adeb input=95ec4f8d9c23530a]*/
{
    Py_ssize_t size;
    const char *encoded = PyUnicode_AsUTF8AndSize((PyObject *) x, &size);
    if (encoded == NULL) {
        return NULL;
    }

    PyObject *decoded = PyUnicode_DecodeUnicodeEscape(encoded, size, NULL);
    if (decoded == NULL) {
        return NULL;
    }

    int n = 1 + (encoding != NULL) + (errors != NULL);
    PyObject *new_args = PyTuple_New(n);
    if (new_args == NULL) {
        Py_DECREF(decoded);
        return NULL;
    }

    PyTuple_SET_ITEM(new_args, 0, decoded);
    if (encoding != NULL) {
        PyTuple_SET_ITEM(new_args, 1, encoding);
    }
    if (errors != NULL) {
        PyTuple_SET_ITEM(new_args, 2, errors);
    }

    PyDecodedObject *str = (PyDecodedObject *) PyUnicode_Type.tp_new(type, new_args, NULL);
    if (str == NULL) {
        Py_DECREF(new_args);
        return NULL;
    }

    Py_DECREF(new_args);
    str->raw = (PyUnicodeObject *) Py_NewRef(x);
    return (PyObject *) str;
}

static PyObject *
DecodedConcrete_repr(PyObject *s)
{
    PyObject *left = NULL, *repr = NULL, *right = NULL, *leftrepr = NULL;
    PyObject *final = NULL;
    
    left = PyUnicode_FromString("DecodedConcrete(");
    if (left == NULL) {
        goto exit;
    }

    repr = PyUnicode_Type.tp_repr(s);
    if (repr == NULL) {
        goto exit;
    }

    right = PyUnicode_FromString(")");
    if (right == NULL) {
        goto exit;
    }

    leftrepr = PyUnicode_Concat(left, repr);
    if (leftrepr == NULL) {
        goto exit;
    }

    final = PyUnicode_Concat(leftrepr, right);

exit:
    Py_XDECREF(left);
    Py_XDECREF(repr);
    Py_XDECREF(right);
    Py_XDECREF(leftrepr);
    return final;
}

static void
DecodedConcrete_dealloc(PyObject *self)
{
    Py_XDECREF(((PyDecodedObject *)self)->raw);
    PyUnicode_Type.tp_dealloc(self);
}

static PyMemberDef DecodedConcrete_members[] = {
    {"raw", Py_T_OBJECT_EX, offsetof(PyDecodedObject, raw), 0, "Raw"},
    {NULL}
};

PyTypeObject _PyDecodedConcrete_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "DecodedConcrete",
    .tp_doc = PyDoc_STR("Decoded object"),
    .tp_basicsize = sizeof(PyDecodedObject),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE |
        Py_TPFLAGS_UNICODE_SUBCLASS |
        _Py_TPFLAGS_MATCH_SELF,
    .tp_new = DecodedConcrete_new,
    .tp_members = DecodedConcrete_members,
    .tp_repr = DecodedConcrete_repr,
    .tp_dealloc = DecodedConcrete_dealloc,
};

PyStatus
_PyDecoded_InitTypes(PyInterpreterState *interp)
{
    _PyDecodedConcrete_Type.tp_base = &PyUnicode_Type;
    if (_PyStaticType_InitBuiltin(interp, &_PyDecodedConcrete_Type) < 0) {
        return _PyStatus_ERR("Can't initialize builtin type");
    }
    return _PyStatus_OK();
}

void
_PyDecoded_FiniTypes(PyInterpreterState *interp)
{
    _PyStaticType_FiniBuiltin(interp, &_PyDecodedConcrete_Type);
}

PyObject *
_PyDecodedConcrete_Create(PyObject *s) {
    if (!PyUnicode_CheckExact(s)) {
        goto error;
    }

    PyObject *res = PyObject_CallFunction((PyObject *) &_PyDecodedConcrete_Type, "O", s);
    if (res == NULL) {
        goto error;
    }

    return res;

error:
    Py_DECREF(s);
    return NULL;
}
