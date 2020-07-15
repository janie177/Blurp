#include "FileReader.h"

namespace blurp
{
	FileReader::FileReader(const std::string& a_Path)
	{
		this->m_Path = a_Path;
		m_Eof = false;
		m_Size = -1;
		m_IsOpen = false;
	}

	bool FileReader::Open()
	{
		m_File.open(m_Path, std::ios::in);
		if (m_File.good())
		{
			//Find size.
			m_Size = static_cast<std::size_t>(m_File.seekg(0, std::ifstream::end).tellg());
			m_File.seekg(0, std::ifstream::beg);
			m_IsOpen = true;
			return true;
		}

		return false;
	}

	bool FileReader::Close()
	{
		if (m_IsOpen)
		{
			m_File.close();
			m_IsOpen = false;
			m_Eof = false;
			m_Size = -1;
			return m_File.good();
		}
		return false;
	}

	//Read the next line as long as there's lines available.
	bool FileReader::NextLine()
	{
		while (!m_Eof)
		{
			m_Eof = std::getline(m_File, m_CurrentLine).eof();
			if (!m_CurrentLine.empty())
			{
				return true;
			}
		}
		return false;
	}

	std::string FileReader::GetCurrentLine() const
	{
		return m_CurrentLine;
	}

	std::ifstream& FileReader::GetFile()
	{
		return m_File;
	}

	std::unique_ptr<char[]> FileReader::ToArray()
	{
		if (m_Size <= 0)
		{
			return nullptr;
		}

		std::unique_ptr<char[]> array = std::unique_ptr<char[]>(new char[m_Size + 1]);
		char c;
		std::uint32_t index = 0;
		while (m_File.get(c) && index < m_Size)
		{
			array[index] = c;
			index++;
		}

		//null terminate at the end of the source.
		array[index] = '\0';

		return array;
	}

	//Start stream from the start again.
	void FileReader::Rewind()
	{
		m_File.clear();
		m_File.seekg(0);
		m_Eof = false;
	}

	std::int32_t FileReader::GetPos()
	{
		return static_cast<std::int32_t>(m_File.tellg());
	}

	bool FileReader::SetPos(std::int32_t a_Pos)
	{
		const auto end = m_File.seekg(0, m_File.end).tellg();

		if (a_Pos <= end && a_Pos >= 0)
		{
			m_File.clear();
			m_File.seekg(a_Pos);
			m_Eof = a_Pos >= end;
			return true;
		}

		//Out of range
		return false;
	}

	std::size_t FileReader::GetSize() const
	{
		return m_Size;
	}
}