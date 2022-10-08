(load #p"intcode.lisp")

(let ((rom (make-rom #p"input21.txt")))
    (let ((cpu (make-cpu rom)))
        (cpu-run-ascii cpu :supress-output t))
    nil)
