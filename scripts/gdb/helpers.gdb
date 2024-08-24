define plist
  set var $n = $arg0
  while $n
    print *$n
    set var $n = $n->next
  end
end
