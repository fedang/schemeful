(include "list.lisp")

;; Let*
;; Linear let evaluation

; (let* ((a 1) (b (+ a 1)) ...) body)
;
; ==>
;
; (let ((a 1))
;   (let ((b (+ a 1)))
;     ...
;       body))

(defmacro let* (vs body)
  ((lambdarec f (vs)
     (if (nil? vs)
       body
       (list 'let
             (list (car vs))
             (f (cdr vs)))))
   vs))

;; Multivariate Y combinator
(define Y*
  (lambda (&rest)
    ((lambda (u) (u u))
     (lambda (p)
       (map (lambda (li) (lambda (&rest) (apply (apply li (p p)) &rest))) &rest)))))

;; Letrec

; NOTE: Works only with lambdas as values
;
; (letrec ((a av) (b bv) ...) body)
; ==>
; (let tmp (Y*
;            (lambda (a b ...) av)
;            (lambda (a b ...) bv))
;   (let ((a (car tmp)) (b (cadr tmp)) ...)
;        body)

(define letrec-bindings
  (lambdarec f (vs)
    (if (nil? vs)
      (cons '() '())
      (if (and (symbol? (caar vs)) (nil? (cddar vs)))
        (let ((bs (f (cdr vs))))
          (cons (cons (caar vs) (car bs)) (cons (cadar vs) (cdr bs))))
        (error "Invalid letrec bindings")))))

(define letrec-lambdas
  (lambdarec f (ss es)
    (if (nil? es)
      '()
      (cons
        (list 'lambda ss (car es))
        (f ss (cdr es))))))

(define letrec-lets
  (lambdarec f (ss n body)
    (if (nil? ss)
      body
      (list 'let (list (car ss) (list 'get 'tmp n))
        (f (cdr ss) (+ 1 n) body)))))

(defmacro letrec (vs body)
  (let (bs (letrec-bindings vs))
    (list 'let
          (list 'tmp
            (cons 'Y*
                  (letrec-lambdas (car bs) (cdr bs))))
            (letrec-lets (car bs) 0 body))))

;; Quasiquote
;; TODO!
;(defmacro unquote (&rest)
;  (error "Invalid unquote"))
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
;
;(define qq-expand
;  (lambda ()))
