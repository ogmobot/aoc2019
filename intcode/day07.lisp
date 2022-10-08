(load #p"intcode.lisp")

(defun permutations (l)
    (cond ((null l) nil)
          ((null (cdr l)) (list l))
          (t (loop for element in l
                append (mapcar (lambda (l) (cons element l))
                    (permutations (remove element l)))))))

(let ((rom (make-rom #p"input07.txt")))
    ; part 1
    (format t "~a~%"
        (loop for p in (permutations '(0 1 2 3 4))
            maximizing
                (loop for n from 0 upto 4
                    for cpu = (make-cpu rom)
                    for poke-value = 0 then poke-value
                    do (progn
                        (cpu-run-until cpu '(:input))
                        (cpu-poke-input cpu (nth n p))
                        (cpu-run-until cpu '(:input))
                        (cpu-poke-input cpu poke-value)
                        (cpu-run-until cpu '(:output))
                        (setf poke-value (cpu-peek-output cpu)))
                    finally (return poke-value))
            into max-val
            finally (return max-val)))
    ; part 2
    (format t "~a~%"
        (loop for p in (permutations '(5 6 7 8 9))
            maximizing
                (let ((cpus
                    (loop for n from 0 upto 4
                        for cpu = (make-cpu rom)
                        do (cpu-poke-input cpu (nth n p))
                        collect (cdr (cpu-run-until cpu '(:input))))))
                (cpu-poke-input (car cpus) 0)
                (loop
                    for index = 0 then index
                    for result = (cpu-run-until (nth index cpus) '(:output))
                    while (not (equal (car result) :halt))
                    do (progn
                        (setf index (mod (+ index 1) 5))
                        (cpu-poke-input (nth index cpus) (cpu-peek-output (cdr result))))
                    finally (return (cpu-peek-output (nth 4 cpus)))))
            into max-val
            finally (return max-val))))
