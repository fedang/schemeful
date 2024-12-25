(defmacro a (x) (list 'b x))
(defmacro b (x) (list 'a x))

;; Infinite loop
(a 1)
