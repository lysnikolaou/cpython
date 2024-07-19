/* Interpolation object implementation */

#include "Python.h"
#include "pycore_initconfig.h"    // _PyStatus_OK()
#include "pycore_object.h"        // _PyObject_GC_TRACK
#include "pycore_structseq.h"     // _PyStructSequence_FiniBuiltin()
#include "pycore_stackref.h"      // PyStackRef_AsPyObjectSteal

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
_PyInterpolation_FromStackRefSteal(_PyStackRef *values, Py_ssize_t n)
{
    assert(2 <= n && n <= 4);

    PyObject *interpolation = PyStructSequence_New(&_PyInterpolationConcrete_Type);
    if (interpolation == NULL) {
        goto error;
    }

    PyStructSequence_SET_ITEM(interpolation, 0, PyStackRef_AsPyObjectSteal(values[0]));
    PyStructSequence_SET_ITEM(interpolation, 1, PyStackRef_AsPyObjectSteal(values[1]));
    PyStructSequence_SET_ITEM(interpolation, 2,
        n >= 3 ? PyStackRef_AsPyObjectSteal(values[2]) : Py_NewRef(Py_None));
    PyStructSequence_SET_ITEM(interpolation, 3,
        n >= 4 ? PyStackRef_AsPyObjectSteal(values[3]) : Py_NewRef(Py_None));

    _PyObject_GC_TRACK(interpolation);
    return (PyObject *)interpolation;

error:
    PyStackRef_CLOSE(values[0]);
    PyStackRef_CLOSE(values[1]);
    PyStackRef_XCLOSE(values[2]);
    PyStackRef_XCLOSE(values[3]);
    return NULL;
}