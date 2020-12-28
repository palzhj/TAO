
#ifndef 	_HANDLER_PROXIES_H__
#define _HANDLER_PROXIES_H__

template <class HandlerClass, void(HandlerClass::*HandlerFunc)()>
struct GtkHandlerProxy0{
  static bool Proxy(gpointer p) {
    (static_cast<HandlerClass*>(p)->*HandlerFunc)();
    return false;
  }
};

template <class HandlerClass, class A1, void(HandlerClass::*HandlerFunc)(A1)>
struct GtkHandlerProxy1{
  static bool Proxy(A1 a1, gpointer p) {
    //printf("Class %p: Handler1 called for %p\n",a1,p);
    (static_cast<HandlerClass*>(p)->*HandlerFunc)(a1);
    return false;
  }
};


template <class HandlerClass, class A1, class A2, void(HandlerClass::*HandlerFunc)(A1, A2)>
struct GtkHandlerProxy2{
  static bool Proxy(A1 a1, A2 a2,  gpointer p) {
    //printf("Class %p: Handler2 called for %p\n",a1,p);
    (static_cast<HandlerClass*>(p)->*HandlerFunc)(a1, a2);
    return false;
  }
};


template <class HandlerClass, class A1, class A2, class A3, void (HandlerClass::* HandlerFunc)(A1,A2,A3)>
struct GtkHandlerProxy3{
	static void Proxy(A1 a1, A2 a2, A3 a3, gpointer p) {
		(static_cast<HandlerClass*>(p)->*HandlerFunc)(a1, a2, a3);
	}
};

template <class HandlerClass, class A1, class A2, class A3, class A4, void(HandlerClass::* HandlerFunc)(A1, A2, A3, A4)>
struct GtkHandlerProxy4{
  static void Proxy(A1 a1, A2 a2, A3 a3, A4 a4,  gpointer p) {
    (static_cast<HandlerClass*>(p)->*HandlerFunc)(a1, a2, a3, a4);
  }
};

#define G_MEMBER_CALLBACK0(Class, Handler) \
   G_CALLBACK( (GtkHandlerProxy0<Class, &Class::Handler>::Proxy) )

#define G_MEMBER_CALLBACK1(Class, Handler, A1) \
   G_CALLBACK( (GtkHandlerProxy1<Class, A1, &Class::Handler>::Proxy) )

#define G_MEMBER_CALLBACK2(Class, Handler, A1, A2) \
   G_CALLBACK( (GtkHandlerProxy2<Class, A1, A2, &Class::Handler>::Proxy) )

#define G_MEMBER_CALLBACK3(Class, Handler, A1, A2, A3) \
   G_CALLBACK( (GtkHandlerProxy3<Class, A1, A2, A3, &Class::Handler>::Proxy) )

#define G_MEMBER_CALLBACK4(Class, Handler, A1, A2, A3, A4) \
   G_CALLBACK( (GtkHandlerProxy4<Class, A1, A2, A3, A4, &Class::Handler>::Proxy) )




#endif

