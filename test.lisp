; comment

; generic lambda
;(lambda [x y] (+ x y))

; typed lambda
;(lambda [(x int) (y int)] (+int x y))

;(let [(x int 1) (y 1) (z list '())]
;  (+ x y))


;; Lambda example
(print
  ((lambda (x y) x) "a" "b"))

(print
  (((lambda (x) (x x)) (lambda (x) x)) "it works!"))

