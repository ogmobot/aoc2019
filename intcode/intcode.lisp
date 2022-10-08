(require :uiop)

(defun make-rom (filename)
    ; actually numeric vector
    (with-open-file (stream filename)
        (let ((initial
            (mapcar #'parse-integer (uiop:split-string (read-line stream nil) :separator ","))))
            (make-array (length initial) :initial-contents initial :adjustable t))))

(defun digit-at (value place) ; e.g. (digit-at 3456 100) => 4
    (mod (floor (/ value place)) 10))

(defun make-cpu (program)
    ; structure: (pc memory input-buffer output-buffer relative-base)
    (list 0 (copy-seq program) nil nil 0))

(defun cpu-get-pc (cpu)
    (car cpu))

(defun cpu-set-pc (cpu val)
    (setf (car cpu) val))

(defun stretch (long-vector min-size)
    (if (> min-size (length long-vector))
        (setf long-vector (adjust-array long-vector (list min-size) :initial-element 0))
        long-vector))

(defun cpu-get-memory (cpu addr)
    (setf (cadr cpu) (stretch (cadr cpu) (+ addr 1)))
    (let ((memory (cadr cpu)))
        (aref memory addr)))

(defun cpu-set-memory (cpu addr val)
    (setf (cadr cpu) (stretch (cadr cpu) (+ addr 1)))
    (let ((memory (cadr cpu)))
        (setf (aref memory addr) val)))

(defun cpu-get-offset (cpu)
    (cadddr (cdr cpu)))

(defun cpu-set-offset (cpu val)
    (setf (cadddr (cdr cpu)) val))

(defun cpu-get-value (cpu param arg)
    (case param
        (0 ; indirect
            (cpu-get-memory cpu arg))
        (1 ; immediate
            arg)
        (2 ; offset
            (cpu-get-memory cpu (+ arg (cpu-get-offset cpu))))
        (otherwise ; unrecognised mode
            (error "Invalid mode ~S in instruction at ~S" param (cpu-get-pc cpu)))))

(defun cpu-set-value (cpu param arg val)
    (case param
        (0 ; indirect
            (cpu-set-memory cpu arg val))
        (1 ; immediate
            (error "Can't use immediate mode for instruction at ~S" (cpu-get-pc cpu)))
        (2 ; offset
            (cpu-set-memory cpu (+ arg (cpu-get-offset cpu)) val))
        (otherwise ; unrecognised mode
                (error "Invalid mode ~S in instruction at ~S" param (cpu-get-pc cpu)))))

(defun cpu-inc-pc (cpu)
    (let ((opcode (cpu-get-memory cpu (cpu-get-pc cpu))))
        (cpu-set-pc cpu (+ (cpu-get-pc cpu)
            (case (mod opcode 100)
                ; ((possible-opcode-vals) distance-to-move)
                ((1 2) 4) ; add and multiply
                ((3 4) 2) ; input and output
                ((5 6) 3) ; jnz and jez
                ((7 8) 4) ; less than and equals
                (9     2) ; adjust offset
                (99    0) ; halt
                (otherwise (progn (format t "warning: opcode ~a" opcode) 1)))))))

(defun cpu-poke-input (cpu val)
    (setf (caddr cpu) val))

(defun cpu-input (cpu)
    ; populates the input buffer (3rd element of cpu)
    (setf (caddr cpu) (read)))

(defun cpu-peek-output (cpu)
    (cadddr cpu))

(defun cpu-clear-output (cpu)
    (setf (cadddr cpu) nil))

(defun cpu-output (cpu)
    ; prints the value in the output buffer (4th element of cpu)
    (format t "~a~%" (cadddr cpu)))

(defun cpu-step (cpu)
    (let* ((opcode (cpu-get-memory cpu (cpu-get-pc cpu)))
           (arg1 (cpu-get-memory cpu (+ 1 (cpu-get-pc cpu))))
           (arg2 (cpu-get-memory cpu (+ 2 (cpu-get-pc cpu))))
           (arg3 (cpu-get-memory cpu (+ 3 (cpu-get-pc cpu))))
           (mode1 (digit-at opcode 100))
           (mode2 (digit-at opcode 1000))
           (mode3 (digit-at opcode 10000)))
        (cpu-inc-pc cpu)
        (case (mod opcode 100)
            (1 ; addition
                (cpu-set-value cpu mode3 arg3
                    (+ (cpu-get-value cpu mode1 arg1)
                       (cpu-get-value cpu mode2 arg2)))
                :okay)
            (2 ; multiplication
                (cpu-set-value cpu mode3 arg3
                    (* (cpu-get-value cpu mode1 arg1)
                       (cpu-get-value cpu mode2 arg2)))
                :okay)
            (3 ; input
                (if (null (caddr cpu))
                    ; reset pc so this instruction can be repeated
                    (progn
                        (cpu-set-pc cpu (- (cpu-get-pc cpu) 2))
                        :input)
                    (progn
                        (cpu-set-value cpu mode1 arg1 (caddr cpu))
                        (setf (caddr cpu) nil)
                        :okay)))
            (4 ; output
                (setf (cadddr cpu) (cpu-get-value cpu mode1 arg1))
                :output)
            (5 ; jump-if-true
                (if (/= (cpu-get-value cpu mode1 arg1) 0)
                    (cpu-set-pc cpu (cpu-get-value cpu mode2 arg2)))
                :okay)
            (6 ; jump-if-false
                (if (= (cpu-get-value cpu mode1 arg1) 0)
                    (cpu-set-pc cpu (cpu-get-value cpu mode2 arg2)))
                :okay)
            (7 ; less than
                (cpu-set-value cpu mode3 arg3
                    (if (< (cpu-get-value cpu mode1 arg1)
                           (cpu-get-value cpu mode2 arg2))
                        1
                        0))
                :okay)
            (8 ; equals
                (cpu-set-value cpu mode3 arg3
                    (if (= (cpu-get-value cpu mode1 arg1)
                           (cpu-get-value cpu mode2 arg2))
                        1
                        0))
                :okay)
            (9 ; adjust offset
                (cpu-set-offset cpu
                    (+
                        (cpu-get-value cpu mode1 arg1)
                        (cpu-get-offset cpu)))
                :okay)
            (99 ; end program
                :halt)
            (otherwise ; unrecognised opcode
                :crash))))

(defun cpu-run (cpu)
    (loop
        for result = (cpu-step cpu)
        when (eql result :output) do (cpu-output cpu)
        while (member result '(:okay :output))
        finally (return (cons result cpu))))

(defun cpu-run-until (cpu stop-sigs)
    (loop
        for result = (cpu-step cpu)
        while (not (member result (cons :halt stop-sigs) :test #'eql))
        finally (return (cons result cpu))))
    
(defun cpu-run-ascii (cpu &key supress-output)
    (loop
        for result = (cpu-step cpu)
        when (eql result :output)
            do (let ((output (cpu-peek-output cpu)))
                (if (<= output 255)
                    (if (not supress-output)
                        (write-char (code-char output)))
                    (format t "~a~%" output)))
        when (eql result :input)
            do (cpu-poke-input cpu (char-code (read-char)))
        do (finish-output)
        until (eql result :halt)
        finally (return (cons result cpu))))
