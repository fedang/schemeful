(include "macro.lisp")

(print (quasiquote a))
(define a 1)

(print (quasiquote (unquote a)))

(print (quasiquote (a b (unquote-splicing '(a b c)))))

(print (list* 'a 'b '(c d)))

(let*
  ((a1
     (begin
       (print 1)
       "first"))
  (a2
     (begin
       (print 2 a1)
       "second"))
  (a3
     (begin
       (print 3 a1 a2)
       "third")))
  (print "Ordered" a1 a2 a3))
