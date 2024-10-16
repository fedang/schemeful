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

(defmacro unquote (&rest)
  (error "Invalid unquote outside of quasiquote"))

(defmacro unquote-splicing (&rest)
  (error "Invalid unquote-splicing outside of quasiquote"))

; Implementation based on the one by Kent Dybvig

(defmacro quasiquote (x)
  (let
    ((check
       (lambda (x)
         (lambda (x)
           (unless
             (and (cons? (cdr x)) (nil? (cddr x)))
             (error "Invalid form" x))))))
    (letrec
      ((qq-expand
         (lambda (x)
           (cond
             ((not (cons? x)) (list 'quote x))
             ((symbol= (car x) 'quasiquote)
              (begin
                (check x)
                (qq-expand (qq-expand (cadr x)))))
             ((symbol= (car x) 'unquote)
              (begin
                (check x)
                (cadr x)))
             ((symbol= (car x) 'unquote-splicing)
              (error "invalid context for unquote-splicing"))
             ((and (cons? (car x)) (symbol= (caar x) 'unquote-splicing))
              (begin
                (check (car x))
                (let ((d (qq-expand (cdr x))))
                  (if (and (symbol= (car d) 'quote) (nil? (cdr d)))
                    (cadar x)
                    (list 'append (cadar x) d)))))
             (else
               (let ((a (qq-expand (car x))) (d (qq-expand (cdr x))))
                 (if (cons? d)
                   (if (symbol= (car d) 'quote)
                     (if (and (cons? a) (symbol= (car a) 'quote))
                       (list 'quote (cons (cadr a) (cadr d)))
                       (if (nil? (cadr d))
                         (list 'list a)
                         (list 'list* a d)))
                     (if (or (symbol= (car d) 'list) (symbol= (car d) 'list))
                       (list* (car d) a (cdr d))
                       (list 'list* a d)))
                   (list 'list* a d))))))))
      (qq-expand x))))
