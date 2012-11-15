# Keep a running tally of the versions of ParaView we use as the base
# of the MADAI Workbench. Only the line for the most recent version of
# the MADAI Workbench should be uncommented.

# MADAI Workbench 1.2.2
#export PARAVIEW_COMMIT="1439967260d37255e86d328424597a6e93921a3f"

# MADAI Workbench 1.3.0
#export PARAVIEW_COMMIT="b5095b55545e5098b8b9ef146027e00cf6bd94c3"

# MADAI Workbench 1.4.0
#export PARAVIEW_COMMIT="1a6245cd454493c7aff82d4b460988179498cb79"
export PARAVIEW_COMMIT="v3.98.0-RC2"

echo "Using ParaView version ${PARAVIEW_COMMIT} as base for MADAI Workbench"
