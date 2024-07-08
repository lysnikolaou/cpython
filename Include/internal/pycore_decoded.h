#ifndef Py_INTERNAL_DECODED_H
#define Py_INTERNAL_DECODED_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef Py_BUILD_CORE
#  error "this header requires Py_BUILD_CORE define"
#endif

typedef struct {
    PyUnicodeObject str;
    PyUnicodeObject *raw;
} PyDecodedObject;

extern PyTypeObject _PyDecodedConcrete_Type;

extern PyStatus _PyDecoded_InitTypes(PyInterpreterState *);
extern void _PyDecoded_FiniTypes(PyInterpreterState *);

PyAPI_FUNC(PyObject *) _PyDecodedConcrete_Create(PyObject *s);

#ifdef __cplusplus
}
#endif

#endif
