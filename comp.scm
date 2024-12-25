(include "macro.scm")

(define asm-start
  (lambda ()
    (begin
      (print "bits 64")
      (print "global entry")
      (print "entry:")
      (print "push rbx")
      (print "mov rbx, rdi"))))

(define asm-end
  (lambda ()
    (begin
      (print "pop rbx")
      (print "ret"))))

(letrec
  ((compile
    (lambda (ex)
      (cond
        ((cons? ex) (compile-cons ex))
        ((symbol? ex) (compile-symbol ex))
        ((string? ex) (compile-string ex))
        ((number? ex) (compile-number ex))
        (else
          (error "unexpected")))))
   (compile-cons
    (lambda (ex)
      ))
   (compile-symbol
    (lambda (ex)
      ))
   (compile-string
    (lambda (ex)
      ))
   (compile-number
    (lambda (ex)
      (begin
      ))))
  (define ex '(print "hello"))
  (asm-start)
  (compile ex)
  (asm-end))
