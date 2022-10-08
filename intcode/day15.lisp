(load #p"intcode.lisp")

(defconstant +north+ '(0 . -1))
(defconstant +south+ '(0 . 1))
(defconstant +west+ '(-1 . 0))
(defconstant +east+  '(1 . 0))

(defconstant +instructions+ (list
    (cons +north+ 1)
    (cons +south+ 2)
    (cons +west+ 3)
    (cons +east+ 4)))

(defun neighbour-coords (coord)
    (if coord
        (mapcar
            (lambda (delta) (cons (+ (car coord) (car delta)) (+ (cdr coord) (cdr delta))))
            (list +north+ +south+ +west+ +east+))))

(defun find-path (maze from-location to-location)
    (let ((visited (make-hash-table :test #'equalp))
          (nodes (list from-location))
          (path '()))
        ;(format t "from ~s to ~s~%" from-location to-location)
        (defun valid-neighbours (coord)
            (remove-if (lambda (n) (or
                                        (and (gethash n maze) (char= (gethash n maze) #\#))
                                        (gethash n visited)
                                        (not (gethash n maze))))
                        (neighbour-coords coord)))
        (loop
            while nodes
            for attempt = (pop nodes)
            do (progn
                (push attempt path)
                ;(format t "Path so far: ~s~%" path)
                (setf (gethash attempt visited) t)
                (if (equalp attempt to-location)
                    (return (reverse path))
                    (let ((neighbours (valid-neighbours attempt)))
                        (loop
                            while (not (or neighbours (null path)))
                            do (setf path (cdr path)
                                     attempt (car path)
                                     neighbours (valid-neighbours attempt)))
                            ;do (format t "path: ~s~%attempt: ~s~%neighbours: ~s~%" path attempt neighbours)
                        (setf nodes neighbours)))))))

(defun walk-path (cpu path)
    ; assume robot's currently location is (car path)
    ; and that the path is valid
    ;(format t "Walking from ~s to ~s.~%" (car path) (car (last path)))
    (loop
        while (cdr path)
        ;do (format t "Walking path ~s~%" path)
        do (let ((delta (cons (- (caadr path) (caar path))
                              (- (cdadr path) (cdar path)))))
                (cpu-run-until cpu '(:input))
                ;(format t "Poking ~s~%" (cdr (assoc delta +instructions+ :test #'equalp)))
                (cpu-poke-input cpu (cdr (assoc delta +instructions+ :test #'equalp)))
                ;(format t "~s~%"
                (cpu-run-until cpu '(:input :ouput))
                ;(format t "Output is ~s~%" (cpu-peek-output cpu))
                ;)
                (if (/= (cpu-peek-output cpu) 1)
                    (format t "Path seems invalid!~%"))
                (pop path))))

(defun find-adjacent-tile (maze target)
    ; finds an explored tile adjacent to the target
    (loop
        for n in (neighbour-coords target)
        when (and (gethash n maze) (not (char= (gethash n maze) #\#)))
        do (return n)))

(defun explore-maze (cpu)
    (let ((maze (make-hash-table :test #'equalp))
          (robot '(0 . 0))
          (nodes '())) ; "edge of the map"
        (setf (gethash robot maze) #\.
              nodes (neighbour-coords robot))
        (loop
            while nodes
            for target = (pop nodes)
            for move-to = (find-adjacent-tile maze target)
            ;do (format t "~s~%" maze)
            do (let ((delta (cons (- (car target) (car move-to))
                                  (- (cdr target) (cdr move-to)))))
                (walk-path cpu (find-path maze robot move-to))
                ;(format t "Running INTCODE cpu...~%")
                (cpu-run-until cpu '(:input))
                ;(format t "Poking ~s as input.~%" (cdr (assoc delta +instructions+ :test #'equalp)))
                (cpu-poke-input cpu (cdr (assoc delta +instructions+ :test #'equalp)))
                ;(format t "Running INTCODE cpu...~%")
                (cpu-run-until cpu '(:output))
                ;(format t "Output is ~s~%" (cpu-peek-output cpu))
                (case (cpu-peek-output cpu)
                    (0 ; hit a wall
                        (setf robot move-to
                              (gethash target maze) #\#))
                    (1 ; stepped onto empty tile
                        (setf robot target
                              (gethash target maze) #\.))
                    (2 ; stepped onto oxygen tank
                        (setf robot target
                              (gethash target maze) #\O)))
                ;(draw-maze maze robot)
                (if (/= (cpu-peek-output cpu) 0)
                    (setf nodes
                        (remove-if
                            (lambda (x) (gethash x maze))
                            (union nodes (neighbour-coords target)))))))
        maze))

(defun draw-maze (maze robot)
    (loop
        for k being the hash-key of maze
        minimizing (car k) into min-x
        maximizing (car k) into max-x
        minimizing (cdr k) into min-y
        maximizing (cdr k) into max-y
        finally (loop
            for row from min-y upto max-y
            do (format t "~{~^~a~}~%"
                (loop for col from min-x upto max-x
                    collect (cond 
                        ((equalp robot (cons col row)) #\@)
                        ((gethash (cons col row) maze) (gethash (cons col row) maze))
                        (t #\?)))))))

(defun fill-with-oxygen (maze)
    ; the slow-but-simple method
    (let ((num-tiles (loop
                        for v being the hash-value of maze
                        counting (char= v #\.))))
    (loop
        while (> num-tiles 0)
        for timer from 1
        do (loop
            for k being the hash-key
            using (hash-value v) of maze
            when (char= v #\O)
            do (loop
                for neighbour in (neighbour-coords k)
                do (if (and (gethash neighbour maze) (char= (gethash neighbour maze) #\.))
                    (setf (gethash neighbour maze) #\o
                          num-tiles (- num-tiles 1)))))
        do (loop
            for k being the hash-key
            using (hash-value v) of maze
            when (char= v #\o) do (setf (gethash k maze) #\O))
        ;do (draw-maze maze '(0 . 0))
        ;do (format t "T=~a~%" timer)
        ;do (sleep 0.5)
        finally (return timer))))

(let* ((rom (make-rom #p"input15.txt"))
       (cpu (make-cpu rom))
       (maze (explore-maze cpu))
       (o2-location (loop
                        for k being the hash-key
                        using (hash-value v) of maze
                        when (char= v #\O)
                        do (return k))))
    ;(draw-maze maze '(0 . 0))
    ; part 1
    (format t "~a~%" (length (cdr (find-path maze '(0 . 0) o2-location))))
    ; part 2
    (format t "~a~%" (fill-with-oxygen maze)))
