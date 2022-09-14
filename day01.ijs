#!/usr/bin/env ijconsole
input =: ". > cutopen toJ 1!:1<'input01.txt'

fuel =: (>.&0)@(-&2)@<.@(%&3)

echo +/ (fuel input)
echo +/ +/ ((fuel^:(>:i.100)) input)

exit''
