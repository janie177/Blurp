#pragma once
#include <memory>
#include <string>
#include <fstream>

namespace blurp
{
	/*
     * FileReader can open a file from a string path.
     * It can then read the file line by line, or copy it into a char array.
     */
	class FileReader
    {
	public:
		FileReader(const std::string& a_Path);

		/*
		 * Open the file at the given path.
		 * Returns true if the file was successfully opened.
		 */
		bool Open();

		/*
		 * Close the file.
		 * Returns true if the file was open before and no errors occurred.
		 */
		bool Close();

		/*
		 * Set the position of the file pointer to be the next line (after a \n char).
		 * Returns true if there is a next line.
		 * False is returned when the end of the file is reached.
		 */
		bool NextLine();

		/*
		 * Get the string from the line in the file that the file pointer is currently located at.
		 */
		std::string GetCurrentLine() const;

		/*
		 * Get the raw file stream that belongs to this object.
		 * This is only initialized when Open has been called.
		 */
		std::ifstream& GetFile();

		/*
		 * Store this entire file in an array.
		 * Size is checked to make sure it fits.
		 */
		std::unique_ptr<char[]> ToArray();

		/*
		 * Set the file pointer to the start of the file.
		 */
		void Rewind();

		/*
		 * Get the location of the file pointer in the file.
		 */
		std::int32_t GetPos();

		/*
		 * Set the current position of the file pointer in the file.
		 * If the position is invalid, false is returned.
		 */
		bool SetPos(std::int32_t a_Pos);

		/*
		 * Get the size of the file in bytes.
		 */
		std::size_t GetSize() const;

	private:
		std::ifstream m_File;
		std::string m_Path;
		std::string m_CurrentLine;
		std::size_t m_Size;
		bool m_Eof;
		bool m_IsOpen;
	};
}
