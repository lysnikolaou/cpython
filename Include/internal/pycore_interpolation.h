#ifndef Py_INTERNAL_INTERPOLATION_H
#define Py_INTERNAL_INTERPOLATION_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef Py_BUILD_CORE
#  error "this header requires Py_BUILD_CORE define"
#endif

extern PyStatus _PyInterpolation_InitTypes(PyInterpreterState *);
extern void _PyInterpolation_FiniTypes(PyInterpreterState *);

PyAPI_FUNC(PyObject *) _PyInterpolation_FromStackRefSteal(
    _PyStackRef *values,
    Py_ssize_t n);

#ifdef __cplusplus
}
#endif

#endif
