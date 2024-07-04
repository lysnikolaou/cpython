/* Interpolation object implementation */

#include "Python.h"
#include "pycore_initconfig.h"    // _PyStatus_OK()
#include "pycore_object.h"        // _PyObject_GC_TRACK
#include "pycore_structseq.h"     // _PyStructSequence_FiniBuiltin()

static PyTypeObject _PyInterpolationConcrete_Type;

PyDoc_STRVAR(interpolation__doc__,
"Interpolation object");

static PyStructSequence_Field interpolation_fields[] = {
    {"getvalue", NULL},
    {"expr", NULL},
    {"conv", NULL},
    {"format_spec",  NULL},
    {0}
};

static PyStructSequence_Desc interpolation_desc = {
    "InterpolationConcrete", /* name */
    interpolation__doc__, /* doc */
    interpolation_fields,
    4
};

PyStatus
_PyInterpolation_InitTypes(PyInterpreterState *interp)
{
    /* Init float info */
    if (_PyStructSequence_InitBuiltin(interp, &_PyInterpolationConcrete_Type,
                                      &interpolation_desc) < 0)
    {
        return _PyStatus_ERR("can't init float info type");
    }

    return _PyStatus_OK();
}

void
_PyInterpolation_FiniTypes(PyInterpreterState *interp)
{
    _PyStructSequence_FiniBuiltin(interp, &_PyInterpolationConcrete_Type);
}

PyObject *
_PyInterpolation_Create(PyObject *getvalue, PyObject *expr, PyObject *conv, PyObject *format_spec)
{
    PyObject *interpolation = PyStructSequence_New(&_PyInterpolationConcrete_Type);
    if (interpolation == NULL) {
        goto error;
    }

    PyStructSequence_SET_ITEM(interpolation, 0, getvalue);
    PyStructSequence_SET_ITEM(interpolation, 1, expr);
    PyStructSequence_SET_ITEM(interpolation, 2,
        conv != NULL ? conv : Py_NewRef(Py_None));
    PyStructSequence_SET_ITEM(interpolation, 3,
        format_spec != NULL ? format_spec : Py_NewRef(Py_None));

    _PyObject_GC_TRACK(interpolation);
    return (PyObject *)interpolation;

error:
    Py_DECREF(getvalue);
    Py_DECREF(expr);
    Py_XDECREF(conv);
    Py_XDECREF(format_spec);
    return NULL;
}