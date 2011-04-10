template <class T>
class C {
 public:
  static int weak;
};

template <class T>
int C<T>::weak = 0;
