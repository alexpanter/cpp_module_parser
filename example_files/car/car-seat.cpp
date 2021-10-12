export module car :seat;

export namespace car
{
	enum class SeatType
	{
		leather,
		polyester
	};

	class seat
	{
	public:
		SeatType getSeatType()
		{
			return leather;
		}
	}
}
