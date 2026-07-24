# Host-side tests for the Studio's tooling.
#
# A separate seam from shared/tests/ on purpose: this is host tooling that reads
# image files, not device code, and putting it in shared/tests/ would place
# sandbox concerns inside shipped code (ADR-0006). Stdlib only — no install step.

.PHONY: test

test:
	PYTHONPATH=.:tests python3 -m unittest discover -s tests -t . -p 'test_*.py'
