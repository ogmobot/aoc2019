(load #p"intcode.lisp")

(defconstant +rom+ (make-rom #p"input19.txt"))

(defun scan (x y)
    (let ((cpu (make-cpu +rom+)))
        (cpu-run-until cpu '(:input))
        (cpu-poke-input cpu x)
        (cpu-run-until cpu '(:input))
        (cpu-poke-input cpu y)
        (cpu-run-until cpu '(:output))
        (cpu-peek-output cpu)))

(defun check-width (x y width) ; (x y) is topleft of square
    (loop
        for dx from (- width 1) downto 0
        for dy from 0 upto (- width 1)
        do (if (= 0 (scan (+ x dx) (+ y dy)))
                (return nil))
        finally (return t)))

(defun find-beam (x min-y)
    ; find the smallest y-coord that the beam passes thru
    ; start counting up from min-y
    (loop
        for y from min-y
        while (= 0 (scan x y))
        finally (return y)))

(defun place-square (width)
    ; find x and y coords of top-left of square
    (let ((y 0))
        (loop
            for x from 0
            do (setf y (find-beam (+ x (- width 1)) y))
            until (check-width x y width)
            finally (return (cons x y)))))

; part 1
(format t "~a~%"
    (loop
        for y from 0 upto 49
        summing (loop
            for x from 0 upto 49
            counting (= (scan x y) 1))))

; part 2
(format t "~a~%"
    (let ((result (place-square 100)))
        (+ (* 10000 (car result)) (cdr result))))
