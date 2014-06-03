#include "stdafx.h"
#include "ZenDataSource.h"

namespace ZenDiary
{
	namespace App
	{
		ZenDataSource::ZenDataSource()
		{
			
		}

		ZenDataSource::~ZenDataSource()
		{

		}

		void ZenDataSource::OnRequest(int request_id, const Awesomium::ResourceRequest& request, const Awesomium::WebString& path)
		{
			std::string uri(Awesomium::ToString(request.url().path()));
			
			std::string fullname(m_home_path + uri);

			ZenThreadParam *p = new ZenThreadParam();
			p->object = this;
			p->fullname = fullname;
			p->request_id = request_id;

			_beginthread(&ZenDataSource::ThreadProc, 0, p);			
		}

		ZD_STATUS ZenDataSource::AddMimeType(const std::string &key, const std::string &value)
		{
			m_mime_types[key] = value;
			return ZD_NOERROR;
		}

		const std::string &ZenDataSource::GetMimeType(const std::string &key)
		{
			auto i = m_mime_types.find(Helpers::String::ToLower(key));
			if (i == m_mime_types.end())
			{
				static std::string stub("text/html");
				return stub;
			}

			return i->second;
		}

		void ZenDataSource::ThreadProc(void *pParam)
		{
			if (!pParam)
			{
				return;
			}				

			ZenThreadParam *p = reinterpret_cast<ZenThreadParam*>(pParam);

			if (Helpers::Files::IsFileExist(p->fullname))
			{
				std::string data;
				ZD_STATUS status = Helpers::Files::GetFileContent(p->fullname, data);
				if (ZD_SUCCEEDED(status))
				{
					std::string ext = std::string(".") + Helpers::String::ExtractExtension(p->fullname);
					const std::string &mime_type = p->object->GetMimeType(ext);

					p->object->SendResponse(p->request_id, data.length(),
						reinterpret_cast<unsigned char*>(const_cast<char*>(data.c_str())),
						Awesomium::WSLit(mime_type.c_str()));
				}
			}			

			delete p;
		}		
	}
}