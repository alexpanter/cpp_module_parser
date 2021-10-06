/* comment! */ export module car : door;

export namespace car
{
	class door
	{
	public:
		door() : mIsOpen(false) {}
		bool isOpen() { return mIsOpen; }

	private:
		bool mIsOpen;
	};
}
