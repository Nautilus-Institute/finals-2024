set -e

ARG="$1";
OUTPUT="$2"

BASEPATH=$(dirname "$0")
echo $BASEPATH

$BASEPATH/lua-5.3.5/src/luac -s -o /tmp/.luac.out $ARG
$BASEPATH/lua-5.3.5/src/luac -sb /tmp/.luac.out
mv /tmp/.dump.bin $OUTPUT
cat $ARG
$BASEPATH/lua-5.3.5/src/luac -l -l /tmp/.luac.out
rm -f /tmp/.luac.out
