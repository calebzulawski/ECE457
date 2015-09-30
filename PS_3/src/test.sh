#! mysh
ls >out1
chmod -r out1

#this comment is ignored
echo cat without permissions
cat out1 2>err1
cat out1 2>err1

echo stderr truncated:
cat err1

cat out1 2>>err1

echo stderr appended:
cat err1

#whitespace ignored too
chmod +r out1
cat out1 >out2
cat out1 >out2

echo stdout truncated:
cat out2

cat out1 >>out2
echo stdout appended:
cat out2

cat >out3 <out2
echo stdin to stdout
cat out3