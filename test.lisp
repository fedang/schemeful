; comment

; generic lambda
;(lambda [x y] (+ x y))

; typed lambda
;(lambda [(x int) (y int)] (+int x y))

;(let [(x int 1) (y 1) (z list '())]
;  (+ x y))


;; Lambda example
(define try
  (lambda (x)
    (print (list x "==>" (eval x)))))

(try '((lambda (x y) x) "a" "b"))

(try '(((lambda (x) (x x)) (lambda (x) x)) "it works!"))

;; Define example
(define nil?
  (lambda (x)
    (= (tag? x) (tag? '()))))

(print (list "False is" (nil? 1)))
(print (list "True is" (nil? '())))

;; Y Combinator
(define Y
  (lambda (f)
    ((lambda (g) (g g))
     (lambda (g)
       (f (lambda (x) ((g g) x)))))))

(define fac
  (Y (lambda (f)
       (lambda (n)
         (if (= n 0)
           1
           (* n (f (+ n -1))))))))

(print (list "Factorial" (fac 10)))

(define len
  (Y (lambda (f)
       (lambda (l)
         (if (nil? l)
           0
           (+ 1 (f (cdr l))))))))

(define trylen
  (lambda (x)
    (print (list "Length of" x (len x)))))

(trylen '(a b c d e))
(trylen '(1 2 3))
