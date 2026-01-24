#!/bin/sh
set -e
if test "$CONFIGURATION" = "Debug"; then :
  cd /Users/michal-mik/Projects/agpt-xenon/build
  echo Build\ all\ projects
fi
if test "$CONFIGURATION" = "Release"; then :
  cd /Users/michal-mik/Projects/agpt-xenon/build
  echo Build\ all\ projects
fi
if test "$CONFIGURATION" = "MinSizeRel"; then :
  cd /Users/michal-mik/Projects/agpt-xenon/build
  echo Build\ all\ projects
fi
if test "$CONFIGURATION" = "RelWithDebInfo"; then :
  cd /Users/michal-mik/Projects/agpt-xenon/build
  echo Build\ all\ projects
fi

