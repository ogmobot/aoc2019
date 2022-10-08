(load #p"intcode.lisp")

(defun make-network (rom size)
    (let ((network (mapcar
                        (lambda (val)
                            (let ((cpu (make-cpu rom)))
                                (cpu-poke-input cpu val)
                                (list
                                    :cpu cpu
                                    :input-queue '()
                                    :output-packet '()
                                    :idle nil)))
                        (loop for i from 0 upto (- size 1) collect i))))
    ; computers in this network have structure
    ; (:cpu cpu :input-queue () :output-packet () :idle nil)
    network))

(defun step-network (network nat)
    (let ((idlers 0))
        (loop
            for computer in network
            for index from 0 upto (- (length network) 1)
            do (let ((result (cpu-step (getf computer :cpu))))
                ;(format t "~a ~S " index result)
                (case result
                    (:okay t)
                    (:input (progn
                        (if (getf computer :input-queue) ; input available
                            (progn
                                (cpu-poke-input
                                    (getf computer :cpu)
                                    (car (getf computer :input-queue)))
                                (setf
                                    (getf computer :input-queue)
                                    (cdr (getf computer :input-queue)))
                                (setf (getf computer :idle) nil))
                            (progn
                                (cpu-poke-input (getf computer :cpu) -1)
                                (setf (getf computer :idle) t)))
                        (cpu-step (getf computer :cpu)))) ; guaranteed :okay (in theory)
                    (:output
                        (setf
                            (getf computer :output-packet)
                            (append
                                (getf computer :output-packet)
                                (list (cpu-peek-output (getf computer :cpu))))
                            (getf computer :idle) nil)
                        (if (= 3 (length (getf computer :output-packet)))
                            (let ((destination (car (getf computer :output-packet))))
                                ;(format t "~a ==~a==> ~a~%" index (cdr (getf computer :output-packet)) destination)
                                (if (< destination (length network))
                                    (setf
                                        (getf (nth destination network) :input-queue)
                                        (append
                                            (getf (nth destination network) :input-queue)
                                            (cdr (getf computer :output-packet))))
                                    (setf nat (copy-list (cdr (getf computer :output-packet)))))
                                (setf (getf computer :output-packet) '()))))
                    (otherwise nil)))
            counting (getf computer :idle) into idle-count
            finally (setf idlers idle-count))
    ;(if (> idlers 30)
        ;(format t "~{~^~a~}~%" (mapcar (lambda (c) (if (getf c :idle) 1 0)) network)))
    ;(if nat (format t "NAT: ~a~%" nat))
    (if (= (length network) idlers)
        (progn
            ;(format t "255 ==~a==> 0~%" nat)
            (setf (getf (car network) :input-queue) (copy-list nat))
            (loop
                for computer in network
                do (setf (getf computer :idle) nil))
            (cons nat t)) ; sent nat to address 0
        (cons nat nil)))) ; did not send nat

(let ((rom (make-rom #p"input23.txt")))
    ; part 1
    (let ((network (make-network rom 50)) (nat '()))
        (loop
            for interrupt = (car (step-network network nat))
            while (null interrupt)
            finally (format t "~a~%" (cadr interrupt))))
    ; part 2
    (let ((network (make-network rom 50))
          (nat '())
          (last-y-val nil))
        (loop
            for result = (step-network network nat)
            until (and last-y-val (cdr result) (= (cadar result) last-y-val))
            do (setf nat (car result))
            when (cdr result) do (setf last-y-val (cadar result)))
        (format t "~a~%" last-y-val)))
