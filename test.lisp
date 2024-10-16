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

;; Defmacro example
(defmacro nil? (x)
    (list '= (list 'tag? x) (list 'tag? '())))

(print (list "False is" (nil? 1)))
(print (list "True is" (nil? '())))

;; Y Combinator
(define Y
  (lambda (f)
    ((lambda (g) (g g))
     (lambda (g)
       (f (lambda (&rest) (apply (g g) &rest)))))))

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

;; Macros
(define not
  (lambda (b)
    (if b '() 1)))

(defmacro and (a b)
  (list 'if a b '()))

(defmacro or (a b)
  (list 'if a 1 b))

(define map
  (Y (lambda (m)
      (lambda (f l)
        (if (nil? l)
          '()
          (cons
            (f (car l))
            (m f (cdr l))))))))

;;  Multi recursion
(define Y*
  (lambda (&rest)
    ((lambda (u) (u u))
     (lambda (p)
       (map (lambda (li) (lambda (&rest) (apply (apply li (p p)) &rest))) &rest)))))

(let ((even-odd (Y*
         (lambda (e o)
           (lambda (n) (or (= n 0) (o (+ n -1)))))
         (lambda (e o)
           (lambda (n) (and (not (= n 0)) (e (+ n -1))))))))

  (let ((ev (car even-odd)) (od (car (cdr even-odd))))
      (begin
        (print (list "Even 10" (ev 10)))
        (print (list "Even 11" (ev 11)))
        (print (list "Even 2" (ev 2)))
        (print (list "Odd 4" (od 4)))
        (print (list "Odd 17" (od 17)))
        (print (list "Odd 3" (od 3))))))


(defmacro s (x y)
  (list 'lambda (list x) (list 'lambda (list y) (list '+ x y))))

(print (s i j))

(define Y
  (lambda (f)
    ((lambda (g) (g g))
     (lambda (g)
       (f (lambda (&rest) (apply (g g) &rest)))))))

(defmacro lambdarec (rec args body)
  (list 'Y (list 'lambda (list rec) (list 'lambda args body))))

(define x
  (lambdarec F (arg1 arg2)
    (F (error "aaa"))))

;  (letrec
;    ((even (lambda (n) (or (= n 0) (odd (+ n -1)))))
;     (odd  (lambda (n) (and (not (= n 0)) (even (+ n -1))))))
;      (begin
;        (print (list "Even 10" (even 10)))
;        (print (list "Even 11" (even 11)))
;        (print (list "Even 2" (even 2)))
;        (print (list "Odd 4" (odd 4)))
;        (print (list "Odd 17" (odd 17)))
;        (print (list "Odd 3" (odd 3)))))
;

;
;(defmacro quasiquote (&rest)
;  (cons 'list ((lambdarec q (l)
;    (if (nil? l)
;      '()
;      (if (cons? l)
;        (if (and (symbol? (car l)) (= (car l) 'unquote))
;          (cdr l)
;          (list 'list (cons (q (car l)) (q (cdr l)))))
;        (list 'quote l))))
;   &rest)))
;
;(print (quasiquote (a b c)))

(print "hello" "my" "name" "is")


(print 'hello)
;(print (quasiquote a))
;
;(letrec ((a (lambda (n) n)) (b (lambda (x) x))) a)
