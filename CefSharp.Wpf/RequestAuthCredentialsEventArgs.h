using namespace System;
namespace CefSharp
{
	namespace Wpf
	{
		public ref class RequestAuthCredentialsEventArgs: public EventArgs
		{
		private:
			bool isProxy;
			Uri^ serverAddress;
			String^ realm;
			String^ username;
			String^ password;
			bool isSuccessful;
		public:
			RequestAuthCredentialsEventArgs(Uri^ serverAddress, bool isProxy, String^ realm){
				this->serverAddress = serverAddress;
				this->isProxy = isProxy;
				this->realm = realm;
			}

			virtual property bool IsProxy
			{
				bool get() { return isProxy; }
			}
			virtual property Uri^ ServerAddess
			{
				Uri^ get() { return serverAddress; }
			}
			virtual property String^ Realm
			{
				String^ get() { return realm; }
			}
			virtual property String^ Username
			{
				String^ get() { return username; }
				void set(String^ value) { username = value; }
			}
			virtual property String^ Password
			{
				String^ get() { return password; }
				void set(String^ value) { password = value; }
			}
			virtual property bool IsSuccessful
			{
				bool get() { return isSuccessful; }
				void set(bool value) { isSuccessful = value; }
			}
		};
	}
}