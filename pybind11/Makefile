PYTHON ?= python3
VENV   := .venv
PY     := $(VENV)/bin/python
SUFFIX := $(shell $(PYTHON)-config --extension-suffix)
MODULE := example$(SUFFIX)

CXXFLAGS := -O2 -Wall -shared -std=c++17 -fPIC

$(MODULE): example.cpp | $(VENV)
	$(CXX) $(CXXFLAGS) $$($(PY) -m pybind11 --includes) example.cpp -o $@

$(VENV):
	$(PYTHON) -m venv $(VENV)
	$(PY) -m pip install --quiet --upgrade pip pybind11

test: $(MODULE)
	$(PY) test.py

clean:
	rm -rf $(VENV) example*.so __pycache__

.PHONY: test clean
