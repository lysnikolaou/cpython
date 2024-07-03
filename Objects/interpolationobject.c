#include "Python.h"
#include "pycore_object.h"        // _PyObject_GC_TRACK

PyDoc_STRVAR(interpolation__doc__,
"Interpolation object");

static PyStructSequence_Field interpolation_fields[] = {
    {"lambda", NULL},
    {"raw", NULL},
    {"conversion", NULL},
    {"format_spec",  NULL},
    {0}
};

static PyStructSequence_Desc interpolation_desc = {
    "interpolation", /* name */
    interpolation__doc__, /* doc */
    interpolation_fields,
    4
};

static PyTypeObject *InterpolationType = NULL;


PyObject *
_PyTagString_CreateInterpolation(PyObject *lambda, PyObject *raw, PyObject *conversion, PyObject *format_spec)
{
    if (InterpolationType == NULL) {
        InterpolationType = PyStructSequence_NewType(&interpolation_desc);
        if (InterpolationType == NULL) {
            goto error;
        }
    }

    PyObject *interpolation = PyStructSequence_New(InterpolationType);
    if (interpolation == NULL) {
        goto error;
    }

    PyStructSequence_SET_ITEM(interpolation, 0, lambda);
    PyStructSequence_SET_ITEM(interpolation, 1, raw);
    PyStructSequence_SET_ITEM(interpolation, 2, conversion);
    PyStructSequence_SET_ITEM(interpolation, 3, format_spec);

    _PyObject_GC_TRACK(interpolation);
    return (PyObject *)interpolation;

error:
    Py_DECREF(lambda);
    Py_DECREF(raw);
    Py_DECREF(conversion);
    Py_DECREF(format_spec);
    return NULL;
}