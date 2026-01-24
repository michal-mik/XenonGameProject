#!/bin/sh
set -e
if test "$CONFIGURATION" = "Debug"; then :
  cd /Users/michal-mik/Projects/agpt-xenon/build/game
  /opt/homebrew/bin/cmake -E copy_directory /Users/michal-mik/Projects/agpt-xenon/graphics /Users/michal-mik/Projects/agpt-xenon/build/game/Debug/graphics
fi
if test "$CONFIGURATION" = "Release"; then :
  cd /Users/michal-mik/Projects/agpt-xenon/build/game
  /opt/homebrew/bin/cmake -E copy_directory /Users/michal-mik/Projects/agpt-xenon/graphics /Users/michal-mik/Projects/agpt-xenon/build/game/Release/graphics
fi
if test "$CONFIGURATION" = "MinSizeRel"; then :
  cd /Users/michal-mik/Projects/agpt-xenon/build/game
  /opt/homebrew/bin/cmake -E copy_directory /Users/michal-mik/Projects/agpt-xenon/graphics /Users/michal-mik/Projects/agpt-xenon/build/game/MinSizeRel/graphics
fi
if test "$CONFIGURATION" = "RelWithDebInfo"; then :
  cd /Users/michal-mik/Projects/agpt-xenon/build/game
  /opt/homebrew/bin/cmake -E copy_directory /Users/michal-mik/Projects/agpt-xenon/graphics /Users/michal-mik/Projects/agpt-xenon/build/game/RelWithDebInfo/graphics
fi

