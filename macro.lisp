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

(define letrec-vars
  (lambdarec f (ss n tmp)
    (if (nil? ss)
      '()
      (cons
        (list (car ss) (list 'get tmp n))
        (f (cdr ss) (+ 1 n) tmp)))))


(defmacro letrec (vs body)
  (let ((bs (letrec-bindings vs))
        (tmp (gensym)))
    (list 'let
          (list
            (list tmp
                  (cons 'Y* (letrec-lambdas (car bs) (cdr bs)))))
          (list 'let (letrec-vars (car bs) 0 tmp) body))))

;; Quasiquote
;; TODO!
(defmacro unquote (&rest)
  (error "Invalid unquote"))

(defmacro unquote-splicing (&rest)
  (error "Invalid unquote-splicing"))

; qq-expand algorithm from the paper
; "Quasiquotation in Lisp", Alan Bawden

(defmacro quasiquote (l)
  (letrec
    ((qq-expand
       (lambda (l depth)
         (if (cons? l)
           (cond
             ((symbol= (car l) 'quasiquote)
              (cons 'quasiquote (qq-expand (cdr l) (+ depth 1))))
             ((or (symbol= (car l) 'unquote) (symbol= (car l) 'unquote-splicing))
              (cond
                ((> depth 0)
                 (cons (list 'quote (car l)) (qq-expand (cdr l) (- depth 1))))
                ((and (symbol= 'unquote (car l))
                      (and (not (nil? (cdr l)))
                           (nil? (cddr l))))
                 (cadr l))
                (else
                  (error "Illegal unquoting"))))
             (else
               (append (qq-expand-list (car l) depth)
                       (qq-expand (cdr l) depth))))
           (list 'quote l))))
     (qq-expand-list
       (lambda (l depth)
         (if (cons? l)
           (cond
             ((symbol= (car l) 'quasiquote)
              (list (cons 'quasiquote (qq-expand (cdr l) (+ depth 1)))))
             ((or (symbol= (car l) 'unquote) (symbol= (car l) 'unquote-splicing))
              (cond
                ((> depth 0)
                 (list (cons (list 'quote (car l)) (qq-expand (cdr l) (- depth 1)))))
                ((symbol= (car l) 'unquote)
                 (list 'quote (cdr l)))
                (else
                  (list 'quote (append (cdr l))))))
             (else
               (list (append (qq-expand-list (car l) depth)
                             (qq-expand (cdr l) depth)))))
           (list 'quote (list l))))))
    (qq-expand l 0)))

(print (quasiquote a))
(define a 1)
(print (quasiquote (unquote a)))

(print (quasiquote (a b c)))
