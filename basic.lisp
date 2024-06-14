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
