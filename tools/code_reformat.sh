#!/bin/sh
# "astyle" tool used to reformat the
# source code to a common base
# TODO: Windows version
/usr/bin/astyle --indent=spaces=4 --indent-switches --brackets=break \
                       --convert-tabs --keep-one-line-statements --keep-one-line-blocks \
                       $*
