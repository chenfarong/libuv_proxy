


namespace LibXsMQ
{

class XsMQConnect;
class XsMQProduct;
class XsMQCustomer;

class XsMQConnectListner;



class XsMQConnect
{
public:
	int Open(const char* server,int service,const char* usr_name,const char* usr_pwd);
	int Close();

	int ListnerAdd(XsMQConnectListner* _var);
	int ListnerRemove(XsMQConnectListner* _var);



};




};
