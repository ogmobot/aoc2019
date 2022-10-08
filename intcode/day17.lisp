(load #p"intcode.lisp")

(defun make-char-buffer (cpu)
    ; cpu should be an intcode cpu that is about to produce ASCII output
    ; use hash table until we know how big output is,
    ; then transfer it to buffer (array of chars).
    (let ((output-table (make-hash-table))
          (row 0) (col 0))
        (loop
            for cpu-result = (cpu-run-until cpu '(:output))
            until (eql (car cpu-result) :halt)
            when (eql (car cpu-result) :output)
            do (let ((out-ch (code-char (cpu-peek-output cpu))))
                (if (char= out-ch #\Newline)
                    (setf row (+ row 1)
                          col 0)
                    (setf
                        (gethash (cons row col) output-table) out-ch
                        col (+ col 1))))
            maximizing row into max-rows
            maximizing col into max-cols
            ; return buffer at end of loop
            finally (return
                (let ((buffer
                    (make-array `(,(- max-rows 1) ,max-cols)
                                :element-type 'character)))
                    (loop
                        for k being the hash-keys
                        using (hash-value v) of output-table
                        do (setf (aref buffer (car k) (cdr k)) v))
                    buffer)))))

(defun display-buffer (buffer)
    ; where buffer is an array of chararacters
    (let ((adims (array-dimensions buffer)))
        (loop
            for row from 0 upto (- (car adims) 1)
            do (loop
                for col from 0 upto (- (cadr adims) 1)
                do (format t "~c " (aref buffer row col)))
            do (format t "~%"))))

(defun find-alignment (buffer)
    (let ((adims (array-dimensions buffer)))
        (loop
            for row from 1 upto (- (car adims) 2)
            append (loop
                for col from 1 upto (- (cadr adims) 2)
                when (and (char= (aref buffer row col) #\#)
                          (char= (aref buffer (- row 1) col) #\#)
                          (char= (aref buffer (+ row 1) col) #\#)
                          (char= (aref buffer row (- col 1)) #\#)
                          (char= (aref buffer row (+ col 1)) #\#))
                ; this is an alignment parameter
                collect (* row col)))))

;(let ((test-lines  '("..#.........."
;                     "..#.........."
;                     "#######...###"
;                     "#.#...#...#.#"
;                     "#############"
;                     "..#...#...#.."
;                     "..#####...^.."))
;      (buffer (make-array '(7 13))))
;        (loop
;            for row from 0
;            for line in test-lines
;            do (loop
;                for col from 0
;                for ch across line
;                do (setf (aref buffer row col) ch)))
;        (format t "~s~%" (find-alignment buffer)))
            
(let ((rom (make-rom #p"input17.txt")))
    ; part 1
    (let* ((cpu (make-cpu rom))
           (buffer (make-char-buffer cpu)))
        ;(display-buffer buffer)
        (format t "~a~%"
            (apply '+ (find-alignment buffer))))
    ; part 2
    (let ((cpu (make-cpu rom)))
        (cpu-set-memory cpu 0 2) ; programmable mode
        (cpu-run-ascii cpu :supress-output t)))
