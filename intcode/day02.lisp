(load #p"intcode.lisp")

(let ((rom (make-rom #p"input02.txt")))
    ; part 1
    (let ((cpu (make-cpu rom)))
        (cpu-set-memory cpu 1 12)
        (cpu-set-memory cpu 2 02)
        (format t "~a~%" (cpu-get-memory (cdr (cpu-run cpu)) 0)))
    ; part 2
    (format t "~a~%"
    (loop named outer for noun from 0 upto 99
        do (loop
            for verb from 0 upto 99
            do (let ((cpu (make-cpu rom)))
                (cpu-set-memory cpu 1 noun)
                (cpu-set-memory cpu 2 verb)
                (if (= (cpu-get-memory (cdr (cpu-run cpu)) 0) 19690720)
                    (return-from outer (format nil "~a~a" noun verb))))))))
