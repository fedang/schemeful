(include "basic.lisp")

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
  (lambdarec m (f l)
    (if (nil? l)
      '()
      (cons
        (f (car l))
        (m f (cdr l))))))

(define length
  (lambdarec f (l)
    (if (nil? l)
      0
      (+ 1 (f (cdr l))))))

(define append
  (lambdarec f (a b)
    (if (nil? a)
      b
      (cons (car a) (f (cdr a) b)))))

(define find
  (lambdarec f (l x)
    (if (nil? l)
      '()
      (if (and (= (tag? x) (tag? (car l))) (= x (car l)))
        1
        (f (cdr l) x)))))

