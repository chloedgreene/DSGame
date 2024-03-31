#!/bin/bash
DIR="$( cd "$( dirname "$0" )" && pwd )"
rm -rf ~/.TrenchBroom/games/Blahaj
cp -R "$DIR/trenchbroom_spec" ~/.TrenchBroom/games/Blahaj
echo Only Run this command if you know what your doing!