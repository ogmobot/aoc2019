(load #p"intcode.lisp")

(defun draw-panels (panels)
    (let ((xs (mapcar #'car (mapcar #'car panels)))
          (ys (mapcar #'cdr (mapcar #'car panels)))
          (output (make-array 0 :element-type 'character :fill-pointer 0 :adjustable t)))
        (loop for y from (reduce #'max ys) downto (- (reduce #'min ys) 1)
            do (loop for x from (reduce #'min xs) upto (+ 1 (reduce #'max xs))
                do (vector-push-extend
                    (if (assoc (cons x y) panels :test #'equal)
                        (if (= (cdr (assoc (cons x y) panels :test #'equal)) 1)
                            #\#
                            #\.)
                        #\Space) output))
            do (vector-push-extend #\Newline output))
        output))

(defun run-robot (rom initial-input)
    (let ((cpu (make-cpu rom))
          (x 0)
          (y 0)
          (direction :up)
          (panels (acons '(0 . 0) initial-input '())))
        (loop
            for result = (cpu-run-until cpu '(:input))
            until (eql (car result) :halt)
            do (progn
                (let ((current-panel (assoc (cons x y) panels :test #'equal)))
                    (cpu-poke-input cpu (if current-panel (cdr current-panel) 0)))
                (cpu-run-until cpu '(:output))
                (setf panels (acons (cons x y) (cpu-peek-output cpu) panels))
                (cpu-run-until cpu '(:output))
                (setf direction (if (= (cpu-peek-output cpu) 0)
                    (case direction ;left
                        (:up    :left)
                        (:left  :down)
                        (:down  :right)
                        (:right :up))
                    (case direction ;right
                        (:up    :right)
                        (:right :down)
                        (:down  :left)
                        (:left  :up))))
                (setf x (case direction
                    (:left (- x 1))
                    (:right (+ x 1))
                    (otherwise x)))
                (setf y (case direction
                    (:up (+ y 1))
                    (:down (- y 1))
                    (otherwise y)))))
        panels))

(let ((rom (make-rom #p"input11.txt")))
    ; part 1
    (format t "~a~%"
        ;(draw-panels (run-robot rom 0)))
        (length (remove-duplicates (mapcar #'car (run-robot rom 0)) :test #'equal)))
    ; part 2
    (format t "~a~%" (draw-panels (run-robot rom 1))))

