;; Recursion

(define Y
  (lambda (f)
    ((lambda (g) (g g))
     (lambda (g)
       (f (lambda (&rest) (apply (g g) &rest)))))))

(defmacro lambdarec (rec args body)
  (list 'Y (list 'lambda (list rec) (list 'lambda args body))))

;; Tagging

(define nil-tag (tag? '()))

(define cons-tag (tag? (list '())))

(define number-tag (tag? 42))

(define string-tag (tag? ""))

(define symbol-tag (tag? 'x))

(defmacro nil? (x)
    (list '= (list 'tag? x) 'nil-tag))

(defmacro cons? (x)
    (list '= (list 'tag? x) 'cons-tag))

(defmacro symbol? (x)
    (list '= (list 'tag? x) 'symbol-tag))

(defmacro string? (x)
    (list '= (list 'tag? x) 'string-tag))

(defmacro number? (x)
    (list '= (list 'tag? x) 'number-tag))

;; Booleans

(define nil '())

(define t 1)

(define not
  (lambda (b)
    (if b '() 1)))

(defmacro and (a b)
  (list 'if a b '()))

(defmacro or (a b)
  (list 'if a 1 b))

(defmacro when (a &rest)
  (list 'if a (list* 'begin &rest) '()))

(defmacro unless (a &rest)
  (list 'if a '() (list* 'begin &rest)))

(define cond-list
  (lambdarec f (l)
    (if (nil? l)
      '()
      (let ((b (car (car l)))
            (v (car (cdr (car l)))))
          (list 'if
                (if (and (symbol? b) (= b 'else)) t b)
                v
                (f (cdr l)))))))

; (cond
;   (b v)
;   ...
;   (else v))
(defmacro cond (&rest)
  (cond-list &rest))

(define symbol=
  (lambda (a b)
    (and
      (symbol? a)
      (and
        (symbol? b)
        (= a b)))))

(define print-tag
  (lambda (x)
    (cond
      ((nil? x) (print "nil-tag"))
      ((cons? x) (print "cons-tag"))
      ((symbol? x) (print "symbol-tag"))
      ((string? x) (print "string-tag"))
      ((number? x) (print "number-tag"))
      (else (error "Impossible")))))

(define equal?
  (lambdarec f (a b)
    (cond
      ((not (= (tag? a) (tag? b))) nil)
      ((cons? a)
       (and
         (f (car a) (car b))
         (f (cdr a) (cdr b))))
      (else (= a b)))))
