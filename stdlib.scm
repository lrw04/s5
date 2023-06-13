;;;; TODO: adapt syntax
(set! list (lambda x x))
(set! define
  (syntax (form . values)
    (if (symbol? form)
        (list 'set! form (car values))
        (list 'set! (car form) (cons 'lambda (cons (cdr form) values))))))
(define and
  (syntax l
    (if (null? l)
        #t
        (if (null? (cdr l))
            (car l)
            (list 'if (car l) (cons 'and (cdr l)) #f)))))
(define (bindings->formals bindings)
  (if (null? bindings)
      '()
      (cons (car (car bindings)) (bindings->formals (cdr bindings)))))
(define (bindings->values bindings)
  (if (null? bindings)
      '()
      (cons (car (cdr (car bindings))) (bindings->values (cdr bindings)))))
(define let
  (syntax (bindings . body)
    (cons (cons 'lambda (cons (bindings->formals bindings) body)) 
          (bindings->values bindings))))
