(load #p"intcode.lisp")

(let ((rom (make-rom #p"input25.txt")))
    (let ((cpu (make-cpu rom)))
        (cpu-run-ascii cpu))
    nil)
