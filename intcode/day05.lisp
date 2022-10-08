(load #p"intcode.lisp")

(let ((rom (make-rom #p"input05.txt")))
    ; part 1
    (let ((cpu (make-cpu rom)))
        (setf (caddr cpu) 1)
        (cpu-run cpu))
    ; part 2
    (let ((cpu (make-cpu rom)))
        (setf (caddr cpu) 5)
        (cpu-run cpu)))
