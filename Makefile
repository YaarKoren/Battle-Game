# ===== General Makefile for convenience - not for submitting =====
.PHONY: all simulator algorithm gamemanager clean

all: simulator algorithm gamemanager

simulator:
	$(MAKE) -C Simulator

algorithm:
	$(MAKE) -C Algorithm

gamemanager:
	$(MAKE) -C GameManager

clean:
	$(MAKE) -C Simulator clean
	$(MAKE) -C Algorithm clean
	$(MAKE) -C GameManager clean