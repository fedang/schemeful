;; Recursion
(define Y
  (lambda (f)
    ((lambda (g) (g g))
     (lambda (g)
       (f (lambda (x) ((g g) x)))))))

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
      (let (b (car (car l)))
        (let (v (car (cdr (car l))))
          (list 'if
                (if (and (symbol? b) (= b 'else)) 1 b)
                v
                (f (cdr l))))))))

; (cond
;   (b v)
;   ...
;   (else v))
(defmacro cond (&rest)
  (cond-list &rest))

;; Lists
(defmacro caar (l)
  (list 'car (list 'car l)))

(defmacro cddr (l)
  (list 'cdr (list 'cdr l)))

(defmacro cadr (l)
  (list 'car (list 'cdr l)))

(defmacro cdar (l)
  (list 'cdr (list 'car l)))

(defmacro caddr (l)
  (list 'car (list 'cddr l)))

(defmacro cdddr (l)
  (list 'cdr (list 'cddr l)))

(defmacro cadddr (l)
  (list 'car (list 'cdddr l)))

(define map
  (lambda (f l)
    ((lambdarec m (l)
      (if (nil? l)
        '()
        (cons
          (f (car l))
          (m (cdr l)))))
    l)))

(define length
  (lambdarec f (l)
    (if (nil? l)
      0
      (+ 1 (f (cdr l))))))

(define append
  (lambda (a b)
    ((lambdarec f (a)
      (if (nil? a)
        b
        (cons (car a) (f (cdr a)))))
     a)))

(define find
  (lambda (l x)
    ((lambdarec f (l)
      (if (nil? l)
        '()
        (if (and (= (tag? x) (tag? (car l))) (= x (car l)))
          1
          (f (cdr l)))))
     l)))
