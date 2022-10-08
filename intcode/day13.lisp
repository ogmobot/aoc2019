(load #p"intcode.lisp")

(defun write-to-buffer (buffer x y tile-id)
    (setf (aref buffer x y) tile-id))

(defun draw-buffer (buffer &optional score)
    (let ((dims (array-dimensions buffer)))
        (if score (format t "SCORE: ~5D~%" score))
        (loop
            for row from 0 upto (- (cadr dims) 1)
            do (format t "~{~^~a~}~%"
                (mapcar
                    (lambda (x) (case x
                        (0 #\Space)
                        (1 #\#)
                        (2 #\O)
                        (3 #\=)
                        (4 #\.)))
                    (loop
                        for col from 0 upto (- (car dims) 1)
                        collect (aref buffer col row)))))))

(let ((rom (make-rom #p"input13.txt")))
    ; part 1
    (format t "~a~%"
    (let ((cpu (make-cpu rom))
          (screen (make-array '(42 20)))
          (total 0))
        (loop
            do (let ((x nil) (y nil) (tile-id nil))
                (if (equal (car (cpu-run-until cpu '(:output))) :halt)
                    (progn
                        (draw-buffer screen)
                        (return total)))
                (setf x (cpu-peek-output cpu))
                (cpu-run-until cpu '(:output))
                (setf y (cpu-peek-output cpu))
                (cpu-run-until cpu '(:output))
                (setf tile-id (cpu-peek-output cpu))
                (if (= tile-id 2) (incf total 1))
                (write-to-buffer screen x y tile-id)))))
    ; part 2
    (let ((cpu (make-cpu rom))
          (screen (make-array '(42 20)))
          (ballx 0)
          (paddlex 0)
          (score 0)
          (draw-counter 0))
        (cpu-set-memory cpu 0 2) ; insert quarters
        (format t "~a~%"
        (loop
            for result = (cpu-run-until cpu '(:input :output))
            while (not (equal (car result) :halt))
            do (cond
                ((equal (car result) :input)
                    (cpu-poke-input cpu
                        (cond ((< ballx paddlex) -1)
                              ((> ballx paddlex)  1)
                              (t                  0))))
                ((equal (car result) :output)
                    (let ((x (cpu-peek-output cpu))
                          (y nil)
                          (tile-id nil))
                        (incf draw-counter 1)
                        (cpu-run-until cpu '(:output))
                        (setf y (cpu-peek-output cpu))
                        (cpu-run-until cpu '(:output))
                        (setf tile-id (cpu-peek-output cpu))
                        (if (= x -1)
                            (setf score tile-id)
                            (progn
                                (if (= tile-id 3) (setf paddlex x))
                                (if (= tile-id 4) (setf ballx x))
                                (write-to-buffer screen x y tile-id))))))
            ;when (and (> draw-counter (* 42 20)) (= 0 (mod draw-counter 10))) do (progn
                ;(loop for i from 0 upto 80 do (format t "~%"))
                ;(draw-buffer screen score))
            finally (return score)))))
