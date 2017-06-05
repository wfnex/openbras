# -*- Makefile -*-

# Load all components.
# Filter out all the components that are currently available from the
# list of requirements.
MISSING_COMPONENTS := $(filter-out $(ALL_COMPONENTS), $(REQUIRED_COMPONENTS))
