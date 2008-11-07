#include <nrnpython.h>
#include <stdio.h>
#include <InterViews/resource.h>
#if HAVE_IV
#include <InterViews/session.h>
#endif
#include <nrnoc2iv.h>
#include <nrnpy_reg.h>

extern "C" {
#include <hocstr.h>
void nrnpython_real();
void nrnpython_start(int);
extern int hoc_get_line();
extern HocStr* hoc_cbufstr;
extern FILE* hoc_fin;
extern char* hoc_promptstr;
extern char* neuronhome_forward();
//extern char*(*PyOS_ReadlineFunctionPointer)(FILE*, FILE*, char*);
# if (PY_MAJOR_VERSION >= 2 && PY_MINOR_VERSION > 2)
static char* nrnpython_getline(FILE*, FILE*, char*);
#else
static char* nrnpython_getline(char*);
#endif
extern void rl_stuff_char(int);
extern int nrn_global_argc;
extern char** nrn_global_argv;
void nrnpy_augment_path();
}

static PyThreadState* main_threadstate_;

void nrnpy_augment_path() {
	static int augmented = 0;
	char buf[1024];
	if (!augmented && strlen(neuronhome_forward()) > 0) {
		//printf("augment_path\n");
		augmented = 1;
		sprintf(buf, "sys.path.append('%s/lib/python')", neuronhome_forward());
		assert(PyRun_SimpleString("import sys") == 0);
		assert(PyRun_SimpleString(buf) == 0);	
		sprintf(buf, "sys.path.prepend('')");
		assert(PyRun_SimpleString("sys.path.insert(0, '')") == 0);	
	}
}

void nrnpython_start(int b) {
#if USE_PYTHON
//	printf("nrnpython_start %d\n", b);
	static int started = 0;
	if (b == 1 && !started) {
		Py_Initialize();
		started = 1;
		int i;
		// see nrnpy_reg.h
		for (i=0; nrnpy_reg_[i]; ++i) {
			(*nrnpy_reg_[i])();
		}
		nrnpy_augment_path();
		main_threadstate_ = PyThreadState_GET();
	}
	if (b == 0 && started) {
		Py_Finalize();
		started = 0;
	}
	if (b == 2 && started) {
		int i;
                PySys_SetArgv(nrn_global_argc, nrn_global_argv);
		nrnpy_augment_path();
		PyOS_ReadlineFunctionPointer = nrnpython_getline;
		// Is there a -c "command" or file.py arg.
		for (i=1; i < nrn_global_argc; ++i) {
			char* arg = nrn_global_argv[i];
			if (strcmp(arg, "-c") == 0 && i+1 < nrn_global_argc) {
				PyRun_SimpleString(nrn_global_argv[i+1]);
				break;
			}else if (strlen(arg) > 3 && strcmp(arg+strlen(arg)-3, ".py") == 0) {
				FILE* fp = fopen(arg, "r");
				if (fp) {
					PyRun_AnyFile(fp, arg);
				}else{
					fprintf(stderr, "Could not open %s\n", arg);
				}
				break;
			}
		}
		// In any case start interactive.
//		PyRun_InteractiveLoop(fopen("/dev/tty", "r"), "/dev/tty");
		PyRun_InteractiveLoop(hoc_fin, "stdin");
	}
	if (b == 3 && started) {
#if HAVE_IV
		if (Session::instance()) {
			Session::instance()->quit();
			rl_stuff_char(EOF);
		}
#endif
	}
#endif
}

void nrnpython_real() {
	int retval = 0;
#if USE_PYTHON
	if (!PyThreadState_GET()) {
		//printf("no threadstate\n");
		PyThreadState_Swap(main_threadstate_);
	}
	retval = PyRun_SimpleString(gargstr(1)) == 0;
#endif
	ret(double(retval));
}

# if (PY_MAJOR_VERSION >= 2 && PY_MINOR_VERSION > 2)
static char* nrnpython_getline(FILE*, FILE*, char* prompt) {
#else
static char* nrnpython_getline(char* prompt) {
#endif
	hoc_cbufstr->buf[0] = '\0';
	hoc_promptstr = prompt;
	int r = hoc_get_line();
//printf("r=%d c=%d\n", r, hoc_cbufstr->buf[0]);
	if (r == 1) {
		int n = strlen(hoc_cbufstr->buf) + 1;
		char* p = (char*)PyMem_MALLOC(n);
		if (p == 0) { return 0; }
		strcpy(p, hoc_cbufstr->buf);
		return p;
	}else if (r == EOF) {
		char* p = (char*)PyMem_MALLOC(2);
		if (p == 0) { return 0; }
		p[0] = '\0';
		return p;	
	}
	return 0;
}
