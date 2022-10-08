(load #p"intcode.lisp")

(let ((rom (make-rom #p"input09.txt")))
    ; part 1
    (let ((cpu (make-cpu rom)))
        (cpu-poke-input cpu 1)
        (cpu-run cpu))
    ; part 2
    (let ((cpu (make-cpu rom)))
        (cpu-poke-input cpu 2)
        (cpu-run cpu))) ; be patient...
