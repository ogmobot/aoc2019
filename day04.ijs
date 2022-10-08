#!/usr/bin/env ijconsole

input_lower =: 372037
input_upper =: 905157

NB. Create a table of digits with shape (n 6)
NB. convert to int <- ravel <- convert each to string <- range
input_range =: (".@,.@:":"0) input_lower+i.@>:(input_upper-input_lower)

ascending   =: <./@:(<:/"1)@:(2&(,\))
freqs       =: (+/"1)@:((i.10)&(=/))

valid_p1    =: (ascending *. (>:&2)@(>./)@freqs)
valid_p2    =: (ascending *. (2&e.)@freqs)

echo +/ valid_p1"1 input_range
echo +/ valid_p2"1 input_range

exit''
