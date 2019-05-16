#include "api.h"
#include "base.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <queue>
#include <random>
#include <string>
#include <utility>
#include <vector>

#define RAD(_DEG_) ((_DEG_)*kPi / 180.0)
#define DEG(_RAD_) ((_RAD_)*180.0 / kPi)

using namespace ts20;

const double kPi = 3.14159265358979323846, kEps = 1e-6;
const double kPoisonData[3][9] = {
	{200.0, 180.0, 160.0, 140.0, 120.0, 100.0, 80.0, 60.0, 40.0},	  //  Frames
	{750.0, 450.0, 225.0, 112.50, 56.25, 28.13, 14.06, 7.03, 0.0},	 //  Radius
	{1.250, 1.667, 1.406, 0.804, 0.469, 0.281, 0.176, 0.117, 0.176} };  //  Speed

extern XYPosition start_pos, over_pos;
extern std::vector<int> teammates;
extern int frame;
extern PlayerInfo info;
PlayerInfo prev_info = info;
const XYPosition &pos = info.self.xy_pos, &prev_pos = prev_info.self.xy_pos;
std::random_device rand_dev;
std::minstd_rand pseudo_rand;
int teammate_id[4];  //队友的id,顺序同枚举

//	<---------------------------- Geometry --------------------------->
namespace Geometry
{
	int Sgn(double x);
	XYPosition ToXYPos(const PolarPosition &p);
	PolarPosition ToPolPos(const XYPosition &p);
	double RelAngle(double angle);

	class Vector
	{
	public:
		Vector(double x = 0.0, double y = 0.0) { x_ = x, y_ = y; }
		Vector(const Vector &v) { x_ = v.x_, y_ = v.y_; }
		Vector(const XYPosition &p) { x_ = p.x, y_ = p.y; }
		Vector(const XYPosition &p, const XYPosition &q) { x_ = q.x - p.x, y_ = q.y - p.y; }
		Vector &operator=(const Vector &v)
		{
			x_ = v.x_, y_ = v.y_;
			return *this;
		}
		Vector &operator*=(double p)
		{
			x_ *= p, y_ *= p;
			return *this;
		}
		Vector &operator+=(const XYPosition &p)
		{
			x_ += p.x, y_ += p.y;
			return *this;
		}

		double GetX(void) const { return x_; }
		double GetY(void) const { return y_; }
		double Length(void) const { return sqrt(x_ * x_ + y_ * y_); }
		XYPosition ToXYPos(void) const { return XYPosition{ x_, y_ }; }

	private:
		double x_, y_;
	};
	typedef Vector Point;
	double Distance(double x1, double y1, double x2, double y2);
	double Distance(const XYPosition &p, const XYPosition &q);
	double Distance(const Point &p, const XYPosition &q);
	double Distance(const XYPosition &p, const Point &q);
	double Distance(const Point &p, const Point &q);
	typedef std::pair<Point, Point> Segment;
	bool operator==(const Vector &u, const Vector &v);
	Vector operator+(const Vector &u, const Vector &v);
	Vector operator-(const Vector &u, const Vector &v);
	Vector operator*(double p, const Vector &u);
	Vector operator*(const Vector &u, double p);
	Vector operator/(const Vector &u, double p);
	double DotProd(const Vector &a, const Vector &b);
	double VectorAngle(const Vector &a, const Vector &b);
	Vector Normal(const Vector &a);
	double CrossProd(const Vector &a, const Vector &b);
	double DistToLine(const Point &p, const Point &a, const Point &b);
	double DistToSegment(const Point &p, const Point &a, const Point &b);
	double DistToSegment(const Point &p, const Segment &l);
	bool SegmentIntersects(const Point &a, const Point &b, const Point &c,
		const Point &d);
	bool SegmentIntersects(const Segment &a, const Segment &b);
	Point LineIntersection(const Point &a, const Point &a0, const Point &b,
		const Point &b0);

	int GetAreaID(double x, double y);
	int GetAreaID(const XYPosition &p);
	int GetAreaID(const Point &p);
}  // namespace Geometry

//	<---------------------------- Graph --------------------------->
namespace Graph
{
	const int kMaxVertexNum = (int)5e3, kMaxEdgeNum = (int)2e5, kEdgeBase = 70;
	const int kNeighbor[8][2] = { {1, 0}, {-1, 0}, {0, 1}, {0, -1}, {1, 1}, {-1, 1}, {1, -1}, {-1, -1} };
	const double kMaxDist = 1e5;

	//	Do NOT MODIFY !!!
	const char coded_edges1[] =
		"000v000f00010003000A000e000c000n000k010v010b010n010k010e010f010m010201"
		"080208020b020B020C0203020A020n030A030B030v033b043304320407042Y043f043e"
		"043z043A043a0439040d054G054F054D05060533050w053i050x053f050&"
		"050z0511060z060y060w060h060g0607060l070h070l070k070e070d070m070g080k08"
		"09080b080n080m080C080o090a090r090o090j090m090n090g0a180a0b0a0r0a0o0a0q"
		"0a0j0a0g0a190b0C0b0B0b0D0b0e0c0u0c0d0c0f0c0t0c0v0c320c3a0c3A0d3b0d320d"
		"0v0d0t0d0e0d0k0d0u0d3a0d0l0e0k0e0f0e0l0e0n0f0t0f0k0f0u0f0n0f0v0g0h0g0j"
		"0g0m0g0l0g0y0h190h0H0h0i0h0E0h0z0h0y0h0l0i190i0p0i0o0i0j0i0q0i0E0i0H0i"
		"160j0C0j0o0j0p0j0r0j0m0j0l0k0l0k0n0l0m0m0r0m0n0m0o0o0p0o0r0p0q0p160p0H"
		"0p0E0p190q0r0q160q0H0q190q170r170r160r190s0D0s0A0s0t0s0v0s3b0s320s3a0s"
		"2Y0s3z0s3f0t0u0t320t3b0t3a0u0v0u320u3a0u2Y0v0D0v0A0v3b0w4F0w0x0w0z0w11"
		"0w0&0w3i0w100x4F0x4D0x330x0Q0x0y0x110x0&"
		"0x100x3i0x4G0y0E0y0z0y110y100y0&"
		"0y0Q0z0E0z0T0z0F0A0B0A0D0A3b0B0C0C0D0D3b0E0S0E0T0E0F0E0H0E160F0P0F0@"
		"0F0Y0F0S0F0T0F0!0F0G0F0M0G0M0G0P0G170G180G0H0G160G0@"
		"0H160H170I0L0I4l0I540I4m0I510I530I4k0I0R0I0J0J5E0J5D0J5B0J590J4l0J1F0J"
		"150J0K0J530J120J540J520J130J580J1C0J570K1z0K150K140K120K0V0K0U0K0L0K0Z"
		"0L0V0L0Z0L0Y0L0S0L0R0L0!0L0U0M0Y0M0N0M0P0M0@0M0!0M180M0#0N0O0N0^0N0#"
		"0N0X0N0!0N0@0N0U0O1M0O1k0O0P0O0^0O0#0O0%"
		"0O0X0O0U0O1N0P180P170P190P0S0Q4E0Q100Q0R0Q0T0Q0*"
		"0Q110Q4F0R4l0R4k0R0S0R0Y0R100R0Z0R0*0S0Y0S0T0S0Z0S0@0T0Y0T100T0@"
		"0T110U0V0U0X0U0!0U0Z0U140V1N0V1d0V0W0V1a0V150V140V0Z0W1N0W0$0W0#"
		"0W0X0W0%0W1a0W1d0W1K0X180X0#0X0$0X0^0X0!0X0Z0Y0Z0Y0@0Z0!0!0^0!0@0!0#0#"
		"0$0#0^0$0%0$1K0$1d0$1a0$1N0%0^0%1K0%1d0%1N0%1L0^1L0^1K0^1N0&4K0&4J0&"
		"4I0&330&0*0&110&4G0&4F0&4D0&3i0&4j0*4k0*100*4F0*4G0*4E0*4j0*"
		"3i1011104j114K1133113i125D12131215121F121C121E125E135D13531314131F131C"
		"131E135E131m141a1415141F141E141C141m151z151a151p151b16171619171818191a"
		"1o1a1p1a1b1a1d1a1K1b1z1b1l1b1x1b1u1b1o1b1p1b1w1b1c1b1i1c1i1c1l1c1L1c1M"
		"1c1d1c1K1c1x1d1K1d1L1e1h1e5j1e621e5k1e5*"
		"1e611e5i1e1n1e1f1f6b1f691f671f631f5j1f1r1f1g1f611f1G1f621f601f1H1f661f"
		"651f1J1g1J1g1I1g1G1g1r1g1q1g1h1g1v1h1r1h1v1h1u1h1o1h1n1h1w1h1q1i1u1i1s"
		"1i1j1i1l1i1x1i1w1i1M1i1y1j1s1j1k1j1B1j1y1j1t1j1w1j1x1j1q1k1l1k1B1k1y1k"
		"1A1k1t1k1q1l1M1l1L1l1N1l1o1m621m1E1m1n1m1p1m1D1m1F1m5D1n5j1n5i1n1o1n1u"
		"1n1E1n1v1n1D1o1u1o1p1o1v1o1x1p1u1p1E1p1x1p1F1q1r1q1t1q1w1q1v1q1I1r1s1r"
		"1J1r1I1r1v1s1z1s1y1s1t1s1A1s1I1t1M1t1y1t1z1t1B1t1w1t1v1u1v1u1x1v1I1v1B"
		"1v1w1w1B1w1x1w1y1x1B1x1z1y1z1y1B1z1A1A1B1A1I1C5H1C561C541C531C1D1C1F1C"
		"5E1C5D1C5B1C571C5h1D5i1D1E1D5D1D5E1D5C1D5h1D571E1F1E5h1F541F571G1H1G1J"
		"1G661G651G631G691H681H1I1H611H601H661H651H691I1J1I651J6a1J5A1K1L1K1N1L"
		"1M1M1N1O2b1O1$1O1P1O1R1O2g1O1#1O1!1O231O201P1Z1P231P201P1#1P1$"
		"1P221P1Q1P1W1Q1W1Q1Z1Q2h1Q2i1Q1R1Q2g1Q231R2g1R2h1R2b1R7E1R7F1S8e1S8n1S"
		"7^1S1@1S1T1S1V1S1#1S8u1T9d1T9c1T9b1T991T8o1T8e1T6*"
		"1T1U1T8C1T2c1T8L1T2d1T2M1T2f1T2P1T8M1U2J1U2f1U2e1U2c1U1^1U1%"
		"1U1V1U211V8e1V1^1V211V201V1#1V1@1V221V1%"
		"1W201W1X1W1Z1W231W221W2i1W241X1Y1X271X241X1*1X221X231X1%"
		"1Y2W1Y2u1Y1Z1Y271Y241Y261Y1*1Y1%1Y2X1Z2i1Z2h1Z2j1Z1#1!8n1!8e1!2a1!1@1!"
		"1$1!291!2b1!7W1!7N1!7^1@8n1@8e1@2b1@1#1@201@2a1@7^1@211#201#1$1#211#"
		"231$201$2a1$231$2b1%1^1%1*1%221%211%2e1^2X1^2n1^1&1^2k1^2f1^2e1^211&"
		"2X1&251&241&1*1&261&2k1&2n1&2U1*2i1*241*251*271*221*"
		"21202120232122222722232224242524272526252U252n252k252X2627262U262n262X"
		"262V272V272U272X282j282g2829282b287E287F287W287O287P287p298n298e297&"
		"297X292a297N297E297W297O297w2a2b2a7N2a7W2a7O2b2j2b2g2b7E2b7F2c9c2c9b2c"
		"8D2c2d2c2f2c2P2c2M2c8C2c8L2d9c2d9b2d8D2d2w2d2e2d2P2d2M2d2O2d8L2d8C2e2k"
		"2e2f2e2P2e2O2e2M2e2w2f2J2f2k2f8C2f2z2g2h2g2j2g7E2h2i2i2j2j7E2j7F2k2y2k"
		"2z2k2l2k2n2k2U2l2J2l2v2l2H2l2E2l2y2l2z2l2G2l2m2l2s2m2s2m2v2m2V2m2W2m2n"
		"2m2U2m2H2n2U2n2V2oal2o2r2o8Z2o9I2o8!"
		"2o9F2o9H2o8Y2o2x2o2p2pam2p9N2p9J2p8Z2p2B2p2q2p9H2p2Q2p9I2p9G2p2R2p9M2p"
		"9L2p2T2q2T2q2S2q2Q2q2B2q2A2q2r2q2F2r2B2r2F2r2E2r2y2r2x2r2G2r2A2s2E2s2C"
		"2s2t2s2v2s2H2s2G2s2W2s2I2t2C2t2u2t2L2t2I2t2D2t2G2t2H2t2A2u2v2u2L2u2I2u"
		"2K2u2D2u2A2v2W2v2V2v2X2v2y2w9I2w9a2w2O2w2x2w2z2w2N2w2P2w9b2x8Z2x8Y2x2y"
		"2x2E2x2O2x2F2x2N2y2E2y2z2y2F2y2H2z2E2z2O2z2H2z2P2A2B2A2D2A2G2A2F2A2S2B"
		"2C2B2T2B2S2B2F2C2J2C2I2C2D2C2K2C2S2D2W2D2I2D2J2D2L2D2G2D2F2E2F2E2H2F2S"
		"2F2L2F2G2G2L2G2H2G2I2H2L2H2J2I2J2I2L2J2K2K2L2K2S2K9R2M9g2M9f2M9e2M8C2M"
		"2N2M2P2M9c2M9b2M992M8L2M8X2N8Y2N2O2N9b2N9c2N9a2N8X2N8L2O2P2O8X2P8L2Q2R"
		"2Q2T2Q9M2Q9L2Q9J2Qaj2R9O2R2S2R9H2R9G2R9M2R9L2R9P2S2T2S9L2T9Q2T982U2V2U"
		"2X2V2W2W2X2Y3E2Y3D2Y3w2Y3z2Y3A2Y3a2Y3f2Y392Y3e2Y3v2Zbc2Zbb2Zb92ZaO2ZaN"
		"2Zau2Z3O2Z3N2Z3G2Z3F2Z3J2Z3K2Z3m2Z3q2Z3n2Z3s2Zat2Zas2!3*2!3U2!2#2!3R2!"
		"422!3c2!3l2!3d2!3k2!3S2!412@4S2@4R2@4P2@4K2@4H2@4o2@4e2@4c2@3^2@3$2@"
		"3p2@2%2@4L2@4O2@4M2@3%2@4d2@3h2@3o2@3g2#462#452#432#3Y2#3W2#3l2#3k2#"
		"3d2#3c2#3S2#412#3R2#422#3T2#402#3V2$492$472$462$442$3M2$3L2$3y2$3x2$"
		"3W2$3Z2$452$4a2$3V2$3X2$3!2$3#2%4a2%492%482%3@2%3Z2%3p2%3$2%4e2%3%2%"
		"4d2%3&2%4b2%3!2^3w2^3t2^3v2^3U2^392^3d2^3R2^3A2&3F2&3G2&3I2&3*2&3k2&"
		"3n2&422&3J2*3D2*3C2*3E2*3^2*3e2*3g2*3%2*"
		"3z303O303N303P304c303p303s304d303K313b313c313l3135313j31aR31aS31aQ323a"
		"323b333f333i334G344N344n344H344K344I343h343i344D344E344G353j353l35aR35"
		"aS35aQ353c35aW36aQ36aN36ar36aM36aO363j363m36as36aR37bj37bh37bb37bc37ba"
		"373q373r37bg37bf37bd384P384S384Q383o383r384W384V384T393w393a393A393v39"
		"3x393u393z3a3b3a3A3a3z3a3x3b3c3c3d3c3R3c423c413c3l3d3R3d3U3d3t3d3w3d42"
		"3e3D3e3f3e3z3e3E3e3y3e3B3e3A3f3z3f3A3f3y3g3D3g3h3g3%3g3^"
		"3g3C3g4O3g4L3g4N3h4K3h4I3h4n3h3%"
		"3h4O3h4L3h4N3h4H3i4G3i4D3i4K3i4J3i4H3jaQ3jaO3jaN3jaL3jaR3jaS3jar3k3l3k"
		"423k3*"
		"3k3I3k3F3k3R3laS3l423l3R3l3S3maS3maO3maN3maM3m3n3m3J3mas3mat3mar3m3K3n"
		"3F3n3J3n3G3nas3nat3n3M3n3H3o4V3o4O3o4M3o4p3o4o3o3p3o4d3o4S3o4R3o4P3o4L"
		"3p4O3p4o3p3O3p4d3p4c3p3P3p4L3p4M3qat3qas3q3s3q3K3qbc3qb93qbb3qau3rbt3r"
		"bs3rbi3rbc3rba3r4S3r4Q3r4f3r4T3r4W3rbf3rbg3rbb3r4P3sas3s3O3s3K3s3N3sat"
		"3s3L3s3Q3s3J3t3U3t3T3t3u3t3w3t3Y3t3V3u3U3u3A3u3Y3u3V3u3X3u3x3u3T3u3S3u"
		"3v3v3X3v3A3v3w3v3x3v3Y3w3U3w3A3x3#3x3Z3x3X3x3W3x3Y3x3y3x3A3y3X3y3W3y3#"
		"3y3Z3y3@3y3B3y3z3y3E3z3#3z3D3z3E3z3A3z3B3A3X3B3^3B3@3B3!3B3#3B3&3B3$"
		"3B3C3B3E3C3^3C3&3C3D3C3@3C3!3D3E3D3^3E3#3E3@3F3G3F3I3F3*"
		"3F3J3G443G3J3G3H3G3M3G433H3*3H433H443H463H3M3H403H413H3I3I3*"
		"3I403I433I463Jbc3J443Jas3J3K3J3M3Jat3Kas3K473K3O3K3N3K3L3Kau3Kat3L453L"
		"443L473L483L4a3L3Q3L3M3L3N3M4a3M473M443M433M453N473N3O3N3Q3N483O3P3O4c"
		"3P4c3P4b3P3Q3P483P493Q4c3Q483Q473Q493Q4b3Q4e3R3S3R3U3R3V3R423S3T3S3V3S"
		"3Y3S3W3T3U3T3Y3T3V3T3X3U3Y3V3W3V3Y3W453W3X3W3Z3X3Y3Z4a3Z3!3Z3#3Z3$3!3@"
		"3!3$3!3&3@3#3@3&3@3$3#3&3$3%3$3&3$4O3$4L3$4N3%3^3%4O3%4L3%4N3^3&3^4L3*"
		"403*423*"
		"434041404340464044414241464143414542464344434644454546474b4748474a4849"
		"484b484e494a494e494b4a4e4b4c4b4e4c4M4c4d4c4L4d4O4d4o4d4e4d4L4d4M4e4M4e"
		"4L4fbe4f4h4f4V4f4U4f4W4f4g4fbs4fbt4f4Q4f4p4f4P4f4R4f4s4gbo4g4h4g4s4gbs"
		"4g4v4g4V4g4U4g4i4g4W4g4p4gbt4g4Q4hbw4hbo4h504h4i4h4v4h4s4hbs4h4y4h4!"
		"4h4X4h4Z4h4^4h4V4ibB4ibA4ibo4i4x4i4!4i4X4i4Z4i4y4i4@4i4^4i4%4i4&"
		"4i4v4ibp4i504j4l4j4E4j4F4j4D4j4J4j4I4j4K4j4k4j4n4j4q4k544k4l4k4q4k4t4k"
		"4E4k4m4k4F4k4n4k4D4l4m4l4t4l514l544l4q4l524l4w4m4C4m4x4m514m524m544m4w"
		"4m5c4m584m554m5b4m4t4m594n4F4n4D4n4r4n4I4n4N4n4H4n4J4n4O4n4M4n4o4n4q4n"
		"4E4o4Q4o4H4o4s4o4q4o4M4o4N4o4L4o4p4o4r4o4R4o4S4p4r4p4R4p4Q4p4S4p4s4p4V"
		"4p4W4p4M4p4L4p4U4q4H4q4F4q4E4q4r4q4t4q4u4q4N4q4M4q4I4q4J4r4x4r4s4r4u4r"
		"4M4r4N4r4t4r4v4r4L4r4R4s4U4s4v4s4u4s4R4s4Q4s4S4s4V4t544t504t4u4t4w4t4C"
		"4t4x4t594t5c4t514u4v4u4x4u4z4u4C4u4w4u4y4u504u594v4y4v4z4v4x4v4^"
		"4v504v4!4w594w5c4w4C4w4x4w554w514w524w584w564w4B4x4z4x4C4x4y4x4^"
		"4x5c4y4X4y4C4y4^4y504y4z4y4%4y4@4y4$4y4!4y4Z4y4A4z504z4*4z4^"
		"4z4A4z4C4z5Q4z5N4z5P4A5p4A4$4A4B4A5Q4A5P4A5J4A5N4A5K4A4*4A5n4A5M4A4&"
		"4A5m4A504B5M4B5J4B5L4B5a4B5l4B5b4B594B5F4B5G4B4C4B5I"
		"4C594C5a4C5c4C5F4D4E4D4G4D4J4D4K4D4I4E4F4E4J4E4K4E4I4F4G4G4J4G4K4G4I4H"
		"4I4H4K4H4O4H4N4H4L4I4J4I4O4I4N4J4K4J4O4J4N4J4L4L4S4L4R4L4M4L4O4M4S4M4Q"
		"4M4N4M4R4N4O4O4S4P4Q4P4S4P4W4P4V4P4T4Pbf4Pbg4Qbs4Q4R4Q4W4Q4V4Q4T4R4S4R"
		"4W4R4V4R4T4Tbi4Tbc4T4U4T4W4Tbt4Tbf4Tbs4Tbg4Tbe4Ubi4U4V4Ubt4Ubs4Ubf4Ubg"
		"4Ube4Ubb4V4W4Vbt4Vbs4Wbf4Wbg4Wbe4XbB4Xbw4Xbo4X4Y4X4!4Xbp4X4^"
		"4X5R4X504X5S4XbC4XbA4YbB4YbA4Y5O4Y4Z4Y4@4Y4#4Ybp4Y4%4Y5R4Y5U4Y4&"
		"4Y5S4YbC4Y4*4ZbC4Z4!4Z4@4Z4#4Z4%4Z4&4Z5U4Z4^4Z5R4Z4*4Z5T4!bB4!4@4!4%4!"
		"4#4!4^4!4&4!504!5U4!5T4@bC4@bp4@4#4@4%4@4&4@5U4@4^4@5R4@4*4@5T4#bC4#"
		"bp4#5n4#4$4#5N4#5U4#5O4#5Q4#5R4#5T4#5d4$bD4$bC4$5n4$4%4$5N4$5O4$5Q4$4&"
		"4$5U4$4^4$4*4$5R4$5T4%bp4%5N4%4&4%5Q4%4^4%5O4%4*4^4&4^504^5Q4^5N4^5P4&"
		"4*4&5Q4&5N4&5P4*504*5Q4*5N4*"
		"5P51525154515851555157515c515b51595253525852555257525c525b5259535D5354"
		"535853555357535c535b5359545E545D555M55565558555I555F555H555b555c555a56"
		"5M565h5657565I565F565H565B565E565b565C565c565a575M575h5758575I575F575H"
		"575B575E575C585B585E585b585C585c585a595a595c5a5b5a5F5a5I5a5G5a5L5a5l5b"
		"5L5b5c5b5F5b5I5b5G5c5F5c5I5c5G5dbF5dbE5d5f5d5T5d5S5d5U5dbD5d5e5d5O5d5n"
		"5dbC5d5N5d5P5d5q5ebG5ebE5e5f5ebD5e5q5e5t5e5T5e5S5e5g5e5U5e5n5ebF5ebC5f"
		"bI5fbE5f5&"
		"5f5S5f5g5f5t5f5q5fbD5fbG5fbF5f5w5f5Y5f5V5f5X5gbQ5gbI5gbH5gbF5g6o5g5!"
		"5g5x5g5v5g5Y5g5V5g5X5gbG5g5w5g5Z5g5$5g5#5g5%5g5t5g5&"
		"5h625h5o5h5j5h5C5h5D5h5B5h5H5h5G5h5I5h5i5h5l5i625i5j5i5o5i5r5i5C5i5k5i"
		"5D5i5l5i5B5j5k5j5r5j5*5j625j5o5j605j5u5k6b5k655k5A5k5z5k5v5k5*"
		"5k605k625k5u5k6a5k665k635k695k5r5k675l5D5l5p5l5G5l5L5l5F5l5H5l5M5l5K5l"
		"5m5l5o5l5C5l5B5m5K5m5L5m5J5m5n5m5p5m5P5m5Q5m5O5m5o5m5q5nbD5n5S5n5L5n5J"
		"5n5p5n5P5n5O5n5Q5n5q5n5T5n5U5n5K5o625o5p5o5r5o5s5o5L5o5K5o5G5o5H5p5v5p"
		"5q5p5s5p5K5p5L5p5r5p5t5p5J5qbD5q5S5q5K5q5t5q5s5q5P5q5O5q5Q5q5T5r645r62"
		"5r5s5r5u5r5A5r5v5r675r6a5r5*5s685s625s5^5s5t5s5v5s5x5s5A5s5u5s5w5s5&"
		"5s675t5w5t5x5t5v5t5$5t5&5t5Y5u6b5u5x5u675u6a5u5A5u5v5u635u5*"
		"5u605u665u645u5z5v5x5v5A5v5w5v5$5v6a5wbG5w6n5w5V5w5A5w5$5w5&5w5x5w5#"
		"5w5Z5w5@5w5Y5w5X5w5y5x5V5x5&5x5^5x5$5x5y5x5A5x5Y5y6l5y6d5y6n5y5^5y5%"
		"5y5&5y5z5y5@5y5#5y5!"
		"5y6o5y6m5y5X5y5Y5y5W5z6l5z6e5z6c5z685z695z675z5A5z6n5z645z635z655z6b5z"
		"6d5z605z5*"
		"5A675A685A6a5A605B5C5B5E5B5H5B5I5B5G5C5D5C5H5C5I5C5G5D5E5E5H5E5I5E5G5F"
		"5J5F5G5F5I5F5M5F5L5G5L5G5J5G5H5G5M5H5L5H5J5H5I5H5M5J5K5J5M5J5P5J5Q5K5L"
		"5K5P5K5Q5K5O5L5M5M5Q5NbD5NbC5N5O5N5Q5N5U5N5T5N5R5ObD5ObC5O5P5O5U5O5T5O"
		"5R5P5Q5P5U5P5T5P5R5RbE5RbA5Rbp5R5S5R5U5RbC5RbD5SbF5SbE5SbA5S5T5SbC5SbD"
		"5Sbp5TbE5T5U5TbD5VbR5VbI5V5W5V5Y5VbG5V5$5V5&"
		"5VbH5VbQ5VbF5WbS5WbR5WbI5WbF5W6m5W5X5W5Z5W5!5W5#5W6o5WbG5W5%5WbQ5W5^"
		"5WbH5XbR5XbH5X6m5X5Y5X5Z5X5!5X5#5X6o5X5%5X5$5X5^"
		"5XbQ5YbI5YbF5Y6n5Y5Z5Y5#5Y5!5YbG5Y5$5Y5%5Y5&5ZbR5ZbH5Z5!5Z5#5Z6o5Z5%"
		"5Z5$5Z5^5ZbQ5!bS5!bR5!5@5!6o5!bQ5!6n5!6m5!bH5@5#5@6o5@5%5@5$5@5^5@6n5@"
		"bQ5@6m5@bR5#bH5#6m5#5%5#5$5#5^5#6n5$bG5$5%5$5&5%6m5%5^5%6n5%6o5^6m5^5&"
		"5^6n5^6o5&6n5*6b5*605*625*665*635*655*6a5*695*"
		"67606c606b6061606660636065606a60696067616c6162616661636165616a61696167"
		"616b636d636463666369636a6368646e6465646b6469646a6468646c646d646n656e65"
		"6d6566656b656c666c666b6669666a66686768676a6869686b686d686c696d696c696a"
		"696b6a6c6a6b6b6e6b6c6b6d6c6d6c6e6c6f6c6h6c6l6d6e6d6l6d6h6d6n6d6f6d6m6e"
		"6f6e6h6e6g6e6l6e6q6e6p6e6r6e6i6e6s6f6g6f6h6f6p6f6q6f6r6f6y6f6z6f6A6f6s"
		"6f6B6f6l6f6H6g6l6g6q6g6r6g6h6g6s6g6z6g6B6g6y6g6C6g6t6g6I6g6D6g6K6g6i6h"
		"6l6h6i6h6r6h6s6h6t6h6q6h6u6h6k6h6B6h6C6h6v6ibT6i6k6i6u6i6v6i6j6i6t6i6w"
		"6i6l6i6s6i6x6i6F6ibU6i6r6i6G6ib%6jbW6jbT6j6l6j6x6j6w6jb%"
		"6j6v6j6G6jbU6jb^"
		"6j6k6jc56jbV6jc66j6P6j6O6j6E6kbS6kbU6k6l6k6m6kbT6k6w6kbR6k6x6k6u6kb%"
		"6kbV6k6t6kb^6l6*"
		"6l6m6l6n6lbR6lbU6l6t6mbS6mbR6m6n6m6o6mbQ6mbT6nbQ6n6o6obT6obS6obQ6obR6o"
		"bH6obG6p6q6p6y6p6z6p6H6p6A6p6J6p6Q6p6B6p6S6p6C6p6T6q6z6q6y6q6A6q6I6q6B"
		"6q6J6q6R6q6S6q6K6r6*"
		"6r6H6r6s6r6B6r6A6r6C6r6J6r6K6r6I6r6L6r6E6r6S6s6C6s6B6s6D6s6A6s6E6s6L6s"
		"6J6s6M6s6z6s6T6t6u6t6D6t6C6t6E6t6v6t6L6t6K6t6M6t6N6t6w6t6V6u6v6u6D6u6w"
		"6u6C6u6L6u6K6u6x6u6B6u6V6u6U6vb%6v6w6v6F6v6E6v6x6v6N6v6G6v6M6v6L6wb^"
		"6wbU6w6x6w6G6wb%6w6E6w6N6w6P6wce6xbV6xbU6x6G6xb%6x6P6xb^"
		"6x6O6xc66x6Z6x6Y6y6z6y6H6y6I6y6A6y6Q6y6S6y6B6y6!"
		"6y6C6z6T6z6K6z6C6z6A6z6I6z6H6z6J6z6B6z6R6z6Q6z6S6A706A6T6A6R6A6K6A6B6A"
		"6J6A6I6A6C6A6S6A6H6B6Q6B6H6B6C6B6J6B6K6B6I6B6S6B6R6C6H6C6L6C6J6C6M6C6T"
		"6C6I6D6E6D6L6D6K6D6M6D6F6D6V6D6N6D6X6D6#"
		"6E6U6E6F6E6M6E6L6E6N6E6W6E6O6E6X6Fb%"
		"6F6N6F6M6F6O6F6X6F6L6F6P6F6W6F6Z6F6%"
		"6G6P6Gc56G6O6Gce6G6Z6Gc66G6N6G6Y6Gcf6H796H6I6H6Q6H6R6H6J6H6!"
		"6I6U6I6J6I6R6I6S6I6T6J736J706J6U6J6Q6J6S6J6R6J6T6J6!6K6L6K6U6K6@"
		"6K6S6K6R6L6V6L6U6L6#6L6T6M6^6M6N6M6W6M6X6M6O6M6$6M6#6N766N6$"
		"6N6Z6N6O6N6X6N6W6N6P6N6%6Ocn6Oc56O776O6V6O6P6O6X6O6Z6Oce6O6W6O6&"
		"6PcE6Pco6Pb^6P776P6^6P6Z6Pce6P6Y6Pc56P6&6Pcf6Q6R6Q6S6Q6!"
		"6Q6T6Q796Q7a6R6U6R6S6R6!6R6T6R726S6@6S6T6S6!6S6U6S716T6U6T6!6T716U6@"
		"6U6!6U746U6%6V6W6V6#6V6$6V6X6V6%6V756V6Y6W736W6X6W6$6W6#"
		"6W6Y6W746W756X776X766X6#6X6Y6X6%6X6^6X6Z6X756YcF6Y6Z6Y6%"
		"6Ycn6Yce6Yc56Yco6ZcN6ZcE6Zc56Z6&6Zcn6Z6^6Zce6Z776Zco6!7o6!7h6!7g6!7f6!"
		"716!726!7a6!7b6!7c6@7i6@7g6@786@6#6@726@746@6$6@716@7b6@7a6@7d6@6%6#"
		"7d6#786#766#6$6#736#6%6#726#756#7c6$7l6$7h6$776$6%6$746$756$6^6$766$"
		"7c6%7l6%7k6%736%6^6%756%766%6&6%7e6%746^cN6^7d6^736^6&6^756^776^7e6^"
		"cn6^cE6&cO6&776&766&cn6&cE6&co6&cN6&7e6*786*796*7a6*7f6*7b6*7q6*7i6*"
		"7v6*"
		"d370717079707a7072707b707f7073707c707q7074717o717h717g71747172717a717b"
		"7173717f7178717c727q727i72787273727b727a727c7274727g727f7374737d737i73"
		"7e747r747o747c747d747e747h75cN757v757u7576757e757d757k757l767e767d767m"
		"76cN76cn77c$"
		"77cE77cN777l77cn777u7879787a787b787c797a797f797b797q797c797w797r7a7w7a"
		"7i7a7b7a7f7a7c7a7o7a7q7b7n7b7c7b7g7b7f7b7h7b7q7c7h7c7i7c7q7d7e7d7j7d7t"
		"7d7s7d7A7ecE7e7k7e7l7e7u7e7m7e7t7f7q7f7r7f7x7f7F7g7h7g7q7g7r7g7i7g7y7g"
		"7z7g7j7h7i7h7j7h7p7h7t7h7k7i7j7i7r7i7k7i7u7i7v7j7k7j7t7j7s7j7u7j7A7j7v"
		"7j7y7j7J7k7u7k7t7k7s7k7C7k7z7kcN7l7u7l7t7l7B7l7D7lcN7lc#"
		"7l7M7m7u7mcE7mcO7mc$7md37m7t7mcP7mc%"
		"7mdb7mcn7n7o7n7w7n7x7n7p7n7E7n7q7n7N7n7O7n7P7n7r7n7W7o7p7o7q7o7r7o7y7o"
		"7H7o7s7o7t7p7q7p7x7p7r7p7y7p7F7p7G7p7s7p7O7p7Q7q7w7q7r7q7s7q7x7r7s7r7z"
		"7r7t7r7G7r7B7s7t7s7y7s7A7s7H7s7J7s7G7s7K7t7H7t7A7t7z7t7J7t7K7u7v7u7D7u"
		"7L7ud37v7C7v7B7v7D7v7L7v7A7v7M7vd37v7J7w7x7w7E7w7F7w7N7w7P7w7W7w7^"
		"7x7F7x7O7x7G7x7N7x7P7x7X7x7H7y7B7y7z7y7H7y7G7y7I7y7A7y7R7y7Q7y7P7z7A7z"
		"7I7z7H7z7B7z7S7z7G7z7Q7A7B7A7J7A7K7A7C7A7U7B7C7B7J7B7L7B7D7B7M7B7S7B7V"
		"7Cd47Cc$7C7D7C7L7C7K7C7M7Cd37C7V7Cc#7Cdb7D7M7Dd37D7L7Dc#"
		"7D7V7Dd47D7K7E7F7E7N7E7O7E7W7E7P7E7Y7E7^"
		"7F7O7F7N7F7P7F7X7F7Q7F7Y7G7X7G7H7G7Q7G7P7G7R7G7Y7G7Z7H7R7H7Q7H7S7H7P7H"
		"7T7H7!7H7Y7I827I7J7I7S7I7R7I7T7I7K7I7!7I7Z7I7@7I7#"
		"7I7L7J827J7K7J7S7J7L7J7R7J7!7J7Z7Kdb7K7@7K7L7K7U7K7T7K7M7K7#"
		"7K7V7Ld37Lc#7L7%7L7M7L7V7Ldb7L7T7L7#7Mdl7Mc#7M857M7V7Mdb7Md37M7%"
		"7Mdc7M7$7N7O7N7W7N7X7N7P7N7^7N7*7O807O7*"
		"7O7Z7O7R7O7P7O7X7O7W7O7Y7O7Q7O7&7O7^7P8f7P807P7&"
		"7P7Z7P7W7P7Q7P7Y7P7X7P7R7P7*7Q7&7Q7R7Q7Y7Q7Z7Q7X7Q7*7R7!7R7Y7R7@"
		"7R807S7T7S7!7S7Z7S7@7S7U7S827S7#7T817T7U7T7@7T7!7T7#7T837T7$7T847U7#"
		"7U7@7U7$7U847U7!7U7%7U837U867Vdu7Vd37V857V7%7Vdk7V7$7Vdt7V867Vdl7V7#"
		"7W877W7X7W7^7W7&7W7Y7X817X7Y7X7&7X7*7X807Y8i7Y8f7Y817Y7^7Y7*7Y7&"
		"7Y807Y877Z7!7Z817Z887Z7*7Z7&7!827!817!897!807@8c7@7#7@837@847@7$7@8a7@"
		"897#8l7#8a7#867#7$7#847#837#7%7#8b7$dC7$dk7$8m7$827$7%7$847$867$dt7$"
		"837$8d7%dT7%dD7%dc7%8m7%8c7%867%dt7%857%dk7%8d7%du7^8o7^7&7^7*7^877^"
		"807&817&7*7&877&807&8h7*887*807*877*817*"
		"8g80818087808g81888187818j818b82838289828a8284828b828k8285838i8384838a"
		"83898385838j838k848m848l84898485848b848c8486848k85dU8586858b85dC85dt85"
		"dk85dD86d#"
		"86dT86dk868d86dC868c86dt868m86dD878w878v878u878g878h878p878q878r888x88"
		"8v888n8889888h888j888a888g888q888p888s888b898s898l898a898i898b898h898k"
		"898r8a8w8a8m8a8b8a8j8a8k8a8c8a8l"
		"8a8r8b8A8b8z8b8i8b8c8b8k8b8l8b8d8b8t8b8j8cd#"
		"8c8s8c8i8c8d8c8k8c8m8c8t8cdC8cdT8dd$8d8m8d8l8ddC8ddT8ddD8dd#"
		"8d8t8e8n8e8o8e8p8e8u8e8q8e8F8f8g8f8o8f8p8f8h8f8q8f8u8f8i8f8r8f8F8f8j8g"
		"8D8g8w8g8v8g8j8g8h8g8p8g8q8g8i8g8u8g8n8g8r8h8x8h8n8h8i8h8q8h8p8h8r8h8j"
		"8h8v8h8u8i8j8i8s8i8x8i8t8j8G8j8r8j8s8j8t8j8w8kd#"
		"8k8K8k8J8k8l8k8t8k8s8k8z8k8A8l8t8l8s8l8B8ld#8ldC8mea8mdT8md#"
		"8m8A8mdC8m8J8n8o8n8p8n8q8n8r8o8p8o8u8o8q8o8F8o8r8o8L8o8G8p8x8p8q8p8u8p"
		"8r8p8D8p8F8q8r8q8v8q8u8q8w8q8F8r8w8r8x8r8F8s8t8s8y8s8I8s8H8s8P8tdT8t8z"
		"8t8A8t8J8t8B8t8I8u8F8u8G8u8M8u9g8v8w8v8F8v8G8v8x8v8N8v8O8v8y8w8x8w8y8w"
		"8E8w8I8w8z8x8y8x8G8x8z8x8J8x8K8y8z8y8I8y8H8y8J8y8P8y8K8y8N8y9o8z8J8z8I"
		"8z8H8z8R8z8O8zd#8Aej8A8J8A8I8A8Q8A8S8Ad#8Ae98A9l8A9o8B8J8BdT8Bd$"
		"8Bea8Bei8BeL8B8I8Bd%"
		"8Beb8BeB8C8D8C8L8C8M8C8E8C9g8C9d8C998C9c8D8E8D8F8D8G8D8N8D9k8D9j8D9h8E"
		"9a8E8F8E8M8E9d8E9g8E8G8E9e8E8N8E998E8H8F8G8F8H8F8M8F9e8G8H8G8O8G8I8G8Q"
		"8G8@"
		"8H9l8H8I8H8N8H8P8H9h8H9k8H9i8H9o8I9k8I9h8I8P8I8O8I9o8I9l8I9n8J8K8J8S8J"
		"ei8JeL8Kej8K8R8K8Q8K8S8K9o8K8P8Kei8K9s8K9p8K9r8L8M8L998L9c8L8X8M9k8M9j"
		"8M9g8M9d8M9f8M998M9a8M8@8N9g8N9h8N9k8N8O8N8P8N8#8N8@"
		"8N9d8N9e8O8P8O9h8O9k8O9i8O8Q8O8#8O8$8P8#8P8Q8P9o8P9l8P9n8P8R8P8$"
		"8P9i8Qei8Q9l8Q9o8Q8R8Q8S8Q9s8Q9r8Q9p8Q8T8Rej8Rea8Re98R8$"
		"8R8S8R9l8R9m8R9o8R9s8R9p8R9r8Rei8ReL8S9o8Sei8S9p8S9s8Se98Sej8S9l8S9m8T"
		"8V8T9r8T9q8T9s8TeC8T8U8T9m8T8$8TeK8TeL8T9l8T9n8T8&"
		"8U9q8U8V8UeK8UeJ8Ueq8Uf28U8&"
		"8UeC8U918Uf18UeP8U9r8V9q8Veq8V8W8VeK8VeP8V918VeJ8VeO8Vf18Vf28V8&"
		"8W9E8W958W938W918W9w8W9t8WeP8W9v8WeD8W948Wf18W9x8W9B8W9A8W9C8X9I8X9j8X"
		"8Z8X9a8X9b8X998X9f8X9e8X9g8X8Y8X8@8X8%8Y9I8Y8Z8Y8%8Y8*8Y9a8Y8!8Y9b8Y8@"
		"8Y998Z8!8Z8*8Z9F8Z9I8Z8%8Z9G8Z928!9L8!988!938!9F8!9G8!9I8!928!9Q8!9M8!"
		"9J8!9P8!8*8!9N8@ak8@9b8@8^8@9e8@9j8@9d8@9f8@9k8@9i8@8#8@8%8@9a8@998#"
		"9d8#8&8#8%8#9i8#9j8#9h8#8$8#8^8#9n8#9o8#9m8$9q8$9j8$9h8$8^8$9n8$9m8$"
		"9o8$8&8$9r8$9s8$9i8%ak8%9I8%9b8%8^8%8*8%908%9j8%9i8%9e8%9f8^938^8&8^"
		"908^9i8^9j8^8*8^918^9h8^9n8&9q8&918&908&9n8&9m8&9o8&9r8*9I8*908*928*"
		"988*938*9N8*9Q8*"
		"9F909O909I909190939095909890929094909E909N919491959193919B919E91eq9295"
		"929N929Q92989293929J929F929G929M929K9297939593989394939B939Q949t949894"
		"9B949E9495949A949x949z949w949v9496959E959D959B95969598959w96ad96a8969$"
		"969A969D969C969R969E9697969&"
		"969z97aj979L979R979O979P979N9798979U979K989N989O989Q989G999a999c999f99"
		"9g999e9a9b9a9f9a9g9a9e9b9c9c9f9c9g9c9e9d9e9d9g9d9k9d9j9d9h9e9h9e9f9e9k"
		"9e9j9f9g9f9k9f9j9f9h9h9i9h9k9h9n9h9o9i9j9i9n9i9o9i9m9j9k9k9o9l9m9l9o9l"
		"9s9l9r9l9p9meC9m9n9m9s9m9r9m9p9meL9n9o9n9s9n9r9n9p9p9q9p9s9peL9peC9pei"
		"9pe99q9r9qeL9qeC9qei9qeK9qe99r9s9reC9reK9se99seL9sei9t9u9t9w9teD9teS9t"
		"eP9t9B9tg89tg59tg79t9E9ug79u9D9u9C9u9v9ueD9u9x9ueS9u9y9u9A9uab9uae9ug8"
		"9ug59ueP9vg79v9D9v9w9v9x9v9y9v9A9veS9vab9vae9v9C9v9B9vg89w9E9w9x9weD9w"
		"9A9w9y9weP9w9B9w9C9wae9xg79x9y9x9A9xab9x9C9x9B9xeS9x9D9xg89ygc9yg99y9$"
		"9y9z9yae9yab9yeS9yg89yg79yg59yeD9zg79z9$"
		"9z9A9zae9zab9z9C9z9B9z9D9zad9zeS9zg89A9C9A9B9A9D9A9$"
		"9B9C9B9E9C9D9Cae9Cab9Da89D9E9Dae9Dab9Dad9EeD9E9&"
		"9F9G9F9I9F9M9F9J9F9L9F9Q9F9P9F9N9Gam9G9H9G9M9G9J9G9L9G9Q9G9P9G9N9Hal9H"
		"9I9H9M9H9J9H9L9H9Q9H9P9H9N9Ham9J9K9J9M9J9U9J9P9J9Q9J9O9K9L9Kaj9K9U9Kam"
		"9K9P9Kak9K9Q9K9O9K9R9L9M9Laj9L9U9Lam9L9R9Mam9M9P9M9Q9M9O9N9O9N9Q9O9P9O"
		"9U9Oaj9Oam9P9Q9P9U9Paj9P9R9Q9U9R9^9R9&9R9S9R9U9Raj9Sa89S9#9Sa69Sa39S9^"
		"9S9&9Sa59S9T9S9Z9T9Z9T9#"
		"9Tak9Tal9T9U9Taj9Ta69Uaj9Uak9V9Y9VfV9VgE9VfW9VgB9VgD9VfU9V9%"
		"9V9W9WgN9WgL9WgJ9WgF9WfV9Wa09W9X9WgD9Waf9WgE9WgC9Wag9WgI9WgH9Wai9Xai9X"
		"ah9Xaf9Xa09X9*9X9Y9Xa49Ya09Ya49Ya39Y9^9Y9%9Ya59Y9*9Za39Za19Z9!9Z9#"
		"9Za69Za59Zal9Za79!a19!9@9!aa9!a79!a29!a59!a69!9*9@9#9@aa9@a79@a99@a29@"
		"9*9#al9#ak9#am9#9^9$gE9$g69$ad9$9%9$9&9$ac9$ae9%fV9%fU9%9^9%a39%ad9%"
		"a49%ac9^a39^9&9^a49^a69&a39&ad9&a69*a09*a29*a59*a49*"
		"aha0a1a0aia0aha0a4a1a8a1a7a1a2a1a9a1aha2ala2a7a2a8a2aaa2a5a2a4a3a4a3a6"
		"a4aha4aaa4a5a5aaa5a6a5a7a6aaa6a8a7a8a7aaa8a9a9aaa9ahabgcabgbabgaabfTab"
		"eDabacabaeabg8abg7abg5abeSacfUacadacg7acg8acg6acfTaceSadaeadfTaegcaeeS"
		"afagafaiafgIafgHafgFafgLaggKagahaggDaggCaggIaggHaggLahaiahgHaigMaig4aj"
		"akajamakalalamanhcanaTanaAanapana$ana#ana%"
		"anhxanhyanhwanaoanhbanaYanaxanaXanaZaohxaohwaoaXaoapaohcaoaAaohdaoaDao"
		"hbaoa$aoa#aoaqaoa%aoaxapaqaphdapaDaphcapaAaph%apheaph@aph$"
		"apaGaqhdaqb8aqb6aqb2aqaHaqaFaqaDaqb0aqh%aqa^aqh$aqa*aqh@"
		"aqheaqaGaqb1aqb5aqb4araWaraVaraUaraMaraNaraLaraRaraQaraSarasaravarayas"
		"bcasaSasaNasaMasaLasatasayasaBataSataNatayatauataBatb9atbcaubhaubfauaK"
		"auaFauaBaub9aubaaubcauaEaubkaubgaubdaubjava!"
		"avazavaQavaVavaPavaRavaWavaUavawavayavaMavaLavaNawaPawaNawaUawaVawaTaw"
		"axawazawaZawa!awaYawayawaAawaQaxaPaxazaxaZaxaYaxa!axaAaxa$axa%"
		"axaUaxaTaxa#axaVayaWayaPayaNayaMayaLayazayaBayaCayaVayaUayaQayaRaza!"
		"azaPazaFazaAazaCazaUazaVazaBazaDazaTazaZaAa#"
		"aAaUaAaTaAaDaAaCaAaZaAaYaAa!aAa$"
		"aBbcaBaCaBaEaBaKaBaFaBbhaBbkaBb9aCbiaCb7aCaDaCaFaCaHaCaKaCaEaCaGaCb8aC"
		"bhaDa!"
		"aDaGaDaHaDaFaDb5aDb8aDb0aEaHaEbhaEbkaEaKaEaFaEbdaEb9aEbaaEbgaEbeaEaJaF"
		"aHaFaKaFaGaFb5aFbkaGa^aGaKaGb5aGb8aGaHaGb4aGb1aGb3aGb0aGa*"
		"aGaIaHb8aHb7aHb5aHaIaHaKaHb0aIbxaIbmaIb7aIbqaIb6aIb8aIaJaIb3aIb4aIb2aI"
		"braJbiaJbjaJbqaJbhaJaKaJbeaJbdaJbfaKbhaKbiaKbkaKbaaLaMaLaOaLaRaLaSaLaQ"
		"aLaVaLaWaMaWaMaNaMaRaMaSaMaQaMaVaNaOaOaRaOaSaOaQaOaVaOaWaOaUaPaQaPaSaP"
		"aWaPaVaPaTaPa!aPaZaQaRaQaWaQaVaQaTaRaSaRaWaRaVaRaTaTaUaTaWaTaZaTa!"
		"aUaVaUaZaUa!aUaYaVaWaWa!aXaYaXa!aXa%aXa$aXa@"
		"aXhyaXhxaXhvaXhbaXhCaYaZaYa%aYa$aYa@aYhyaYhvaZhvaZa!aZa%aZa$aZa@aZhya!"
		"hCa@a#a@a%a@hya@hxa@hba@hBa@hCa@hAa@g&a#a$a#hxa#hya#hwa#hba#hca#hfa$a%"
		"a$hba$hca$hfa%hya%hBa%hCa%hAa^i5a^i1a^a&a^b0a^h$a^h#a^h%a^i0a^h*a^h^a^"
		"b5a^b8a&i6a&i5a&i3a&i2a&bla&b7a&a*a&b1a&h$a&b2a&h#a&b4a&h%a&i0a&h*a&h^"
		"a&b6a*i5a*i2a*bla*b0a*b1a*b2a*b4a*i0a*h*a*b6a*b5a*h^a*b7b0b8b0b1b0h$"
		"b0b4b0b2b0h%b0h#b0b5b0b6b1i6b1i5b1i2b1h*"
		"b1b2b1b4b1b6b1b5b1b7b1blb1i0b2i6b2b3b2blb2h*"
		"b2i0b2i5b2bmb3i6b3b4b3b6b3b5b3b7b3blb3h*"
		"b3i5b3bmb3bqb4bqb4bmb4b6b4b5b4b7b4h%b5h%b5b6b5b8b6brb6bqb6b7b6h$"
		"b6blb7b8b7blb7brb8h$b8h%"
		"b9bhb9bab9bcb9bgb9bdb9bfb9bkb9bjbabbbabgbabdbabfbabkbabjbabhbbbcbbbgbb"
		"bdbbbfbbbkbbbjbbbhbdbebdbgbdbjbdbkbdbibebqbebfbebjbebkbebibebtbfbgbfbt"
		"bgbjbgbkbgbibgbtbhbibhbkbibjbibtbjbqbjbkbjbtbli8bli7blh&"
		"blbxblbrblbqbli5bli6blbmblh*"
		"blbubmi8bmbrbmbqbmbubmbxbmi6bnibbnbwbnbzbnbPbnbybni9bnbvbniabnbObni8bn"
		"iMbnbNbobybobwbobBbobxbobrbobsbobpbpbObpbxbpbwbpbAbpbBbpbCbqbrbrbxbrbs"
		"brbubsbxbsbwbsbtbubvbui6bui8bui9bui5bui7bubxbvibbvbPbvbObvi9bvbybvi6bv"
		"i8bvbzbvbwbvi7bviabwbxbwbBbwbybxbBbybzbybPbyi9bybBbyi8bybNbzbObzbPbzbA"
		"bzi9bzbNbziabzi8bAbEbAbBbAbCbAbObBbCbCbObCbFbCbDbCbEbDbObDbIbDbEbDbFbD"
		"bGbEbFbEbMbEbIbEbObEbGbEbNbFbObFbGbFbIbFbHbFbMbGbMbGbHbGbIbGbQbHbMbHbJ"
		"bHbIbHbQbHb#bHbRbHbSbIbMbIbJbIb#bIbLbIbQbJbLbJbKbJbMbJb#bJiDbJb$"
		"bJiEbJiCbJipbJi%bJbNbJb@bJiIbKb@bKbMbKb$bKipbKiDbKi%"
		"bKiCbKiEbKbLbKiIbKiHbKiFbKi@bKi$bKb#bKiobLb#"
		"bLbObLiEbLiDbLiBbLbMbLipbLbNbLinbLivbLiwbLiubLb$"
		"bLiobMbNbMbObMiEbMiDbMiBbMinbMbPbNbObNbPbNinbNijbNiwbNivbNiEbNitbNiBbN"
		"iDbObPbOinbOijbPijbPiabPiMbPiLbPiJbPi9bPinbPiwbPitbQb#bQbRbQbSbQbTbRb#"
		"bRbSbRbTbRbUbSbTbSb!bSbWbSb#bSbUbSb@bSbVbTbUbTbWbTbVbTb!bTb^bTb%bTb&"
		"bUb!bUbVbUbWbUb%bUb^bUb&bUc5bUc6bVb!bVb^bVb&bVbWbVb*"
		"bVc6bVc8bVc5bVc9bVc0bVcfbVcabWb@bWb!bWbXbWb&bWb*bWc0bWb^"
		"bWc1bWbZbWc8bWc9bXb@bXbZbXc1bXc2bXbYbXc0bXc3bXb!bXb*"
		"bXc4bXccbXjcbXjbbXb&bXcdbYj0bYb!"
		"bYjbbYc4bYjcbYc3bYjibYc2bYcdbYbZbYcmbYclbYi*bYjfbYjhbYcbbZjcbZb!bZi*"
		"bZb@bZjbbZc3bZc4bZj0bZc1bZi$bZjibZc0b!b@b!b#b!c0b!c1b!i$b!b$b!b*b@b#b@"
		"b$b@i$b@i%b@j0b@i*b@i#b@ipb#b$b#i%b#i$b#ipb#iDb$i%b$ipb$i$b$iHb$iIb$"
		"iGb$iDb$iCb$iEb$iob%b^b%c5b%c6b%ceb%c7b%cgb%cnb^c6b^c5b^c7b^cfb^c8b^"
		"cgb&b*b&c8b&c7b&c9b&cgb&chb&cfb&cib&cbb&cpb*c9b*c8b*cab*c7b*cbb*cib*"
		"cgb*cjb*c6b*cqc0c1c0cac0c9c0cbc0c2c0cic0chc0cj"
		"c0ckc0c3c0csc1c2c1cac1c3c1c9c1cic1chc1c4c1c8c1csc1crc2jic2jbc2c3c2ccc2"
		"cbc2c4c2ckc2cdc2cjc2cic3jcc3c4c3cdc3cbc3ckc3cmc3jic3jbc4i*"
		"c4cdc4jic4cmc4jbc4clc4jcc4cwc4cvc5c6c5cec5cfc5c7c5cnc6cqc6cpc6chc6c9c6"
		"c7c6cfc6cec6cgc6c8c6coc6cnc7cqc7coc7chc7c8c7cgc7cfc7c9c7cpc7cec8c9c8cg"
		"c8chc8cfc8cpc8coc9cic9cgc9cjc9cqc9cfcacbcacicachcacjcacccacscackcacuca"
		"czcbcrcbcccbcjcbcicbckcbctcbclcbcuccckcccjccclcccucccicccmccctcccwcccB"
		"cdjmcdjccdcDcdcmcdclcdjhcdcwcdckcdcvcdcucdjbcecxcecfcecncecocecgcfcrcf"
		"cgcfcocfcpcfcqcgcIcgcFcgcrcgcncgcpcgcocgcqcgcxchcichcrchcychcpchcocicv"
		"cicscicrciczcicqcjcCcjckcjctcjcucjclcjcAcjczckcLckcAckcwckclckcuckctck"
		"cmckcBclcMclcsclcmclcuclcwclctclcDcljhcmjmcmcMcmcwcmcvcmjhcmcDcmcCcmji"
		"cncOcncxcncqcncocncpcocrcocpcocxcocqcpcycpcqcpcxcpcrcpcGcqcrcqcxcqcGcr"
		"cycrcxcrcJcrcBcscLcsctcsczcscAcscucscBcscKcscvctcIctcuctcActczctcvctcJ"
		"ctcKcuc@"
		"cucMcucLcuczcucvcucBcucCcucwcucKcvcwcvcBcvjhcvjmcvcJcwjlcwjicwjgcwcDcw"
		"cCcwjhcwcMcwjmcxcWcxcVcxcUcxcGcxcHcxcPcxcQcxcRcycXcycVcyczcycHcycJcycA"
		"cycGcycQcycPcycScycBczcSczcLczcCczcAczcIczcBczcHczcKczcRcAc!"
		"cAcWcAcMcAcBcAcJcAcKcAcCcAcLcAcRcBc!"
		"cBcZcBcIcBcCcBcKcBcLcBcDcBcTcBcJcCjhcCcIcCcDcCcKcCcMcCcTcCcScDjycDjgcD"
		"cMcDcLcDjhcDjmcDcTcDjlcEcNcEcOcEcPcEcUcEcQcFcGcFcOcFcPcFcHcFcQcFcUcFcI"
		"cFcRcFc^cFcJcGc$cGcWcGcVcGcJcGcHcGcPcGcQcGcIcGcUcGcNcGcRcHc^"
		"cHcXcHcIcHcQcHcPcHcRcHcJcHcVcHcUcIcJcIcScIcXcIcTcJc&cJc!"
		"cJcRcJcScJcTcJcWcKd2cKd1cKcLcKcTcKcScKcZcKc!cLcTcLcScLc@cLd0cMd1cMc!"
		"cMjlcMjycMjmcMjhcNcOcNcPcNcQcOcPcOcUcOcQcOc^"
		"cOcRcOd3cPcXcPcQcPcUcPcRcPc$cPc^cQcRcQcVcQcUcQcWcQc^cRcWcRcXcRc^"
		"cScTcScYcSd0cSc*cSd7cTjycTcZcTc!cTd1cTc@cTd0cUc^cUc&cUd4cUdccVcWcVc^"
		"cVc&cVcXcVd5cVd6cVcYcWcXcWcYcWc%cWd0cWcZcXcYcXc&"
		"cXcZcXd1cXd2cYcZcYd0cYc*cYd1cYd7cYd2cYd5cYdgcZd1cZd0cZc*cZd9cZd6cZd5c!"
		"jlc!d1c!d0c!d8c!dac!jxc!djc@jyc@jxc@d1c@jlc@d0c@jQc@jNc@jmc@jPc#c$c#"
		"d3c#d4c#c%c#dbc#c^c#dkc$c%c$c^c$c&c$d5c$dec$c*c%c^c%d4c%c&c%d5c%dcc%"
		"ddc%c*c%dlc^c&c^c*c^d4c&c*c&d6c&d0c&ddc&d8c*d0c*d5c*d7c*dec*dgc*ddc*"
		"dhd0ded0d7d0d6d0dgd0dhd1d2d1dad1did1jxd2d9d2d8d2dad2did2d7d2djd2dgd2ds"
		"d3d4d3dbd3dcd3dkd4dcd4dld4ddd4dkd4dmd5d8d5d6d5ded5ddd5dfd5d7d5dod5dnd5"
		"dmd6d7d6dfd6ded6d8d6dpd6ddd6dnd7dad7d8d7dgd7dhd7d9d7drd8d9d8dgd8did8da"
		"d8djd8dpd8dsd9jQd9dad9did9dhd9djd9dsd9jxd9dqd9jydadAdadjdadidajxdadsda"
		"jydadhdbdcdbdkdbdldbdtdbdmdbdvdcdldcdkdcdmdcdudcdndcdvdddedddndddmdddo"
		"dddvdddwdedodedndedpdedmdedqdedxdedvdfdHdfdgdfdpdfdodfdqdfdhdfdxdfdwdf"
		"dydfdzdfdidgdHdgdhdgdpdgdidgdodgdxdgdwdhdydhdidhdrdhdqdhdjdhdzdhdsdijQ"
		"dijydidjdidsdidqdidzdidBdjjNdjjydjdKdjdsdjdBdjjxdjdAdjjQdjjPdkdldkdtdk"
		"dudkdmdkdCdldFdldEdldCdldwdldodldmdldudldtdldvdldndldDdmdFdmdDdmdwdmdn"
		"dmdvdmdudmdodmdEdndodndvdndwdndudndEdodxdodvdodydodFdpdqdpdxdpdwdpdydp"
		"drdpdHdpdzdqdGdqdrdqdydqdxdqdzdqdIdqdAdqdJdrdzdrdydrdAdrdJdrdxdrdBdrdI"
		"drdLdsjHdsjxdsdBdsdAdsdLdsdzdsdKdsdJdtdMdtdudtdCdtdDdtdvdudGdudvdudDdu"
		"dEdudFdvdXdvdUdvdGdvdCdvdEdvdDdvdFdvdMdwdxdwdGdwdNdwdEdwdDdxdKdxdHdxdG"
		"dxdOdxdFdydRdydzdydIdydJdydAdydPdydOdzd!"
		"dzdPdzdLdzdAdzdJdzdIdzdBdzdQdAdHdAdBdAdJdAdLdAdIdAdSdBjQdBjOdBd@"
		"dBdLdBdKdBdSdBdRdBjHdBjPdCd$"
		"dCdMdCdFdCdDdCdEdDdGdDdEdDdMdDdFdEdNdEdFdEdMdEdGdEdVdFdGdFdMdFdVdGdNdG"
		"dMdGdYdGdQdHd!"
		"dHdIdHdOdHdPdHdJdHdQdHdZdHdKdIdXdIdJdIdPdIdOdIdKdIdYdIdZdJd@dJd!"
		"dJdOdJdKdJdQdJdRdJdLdJdZdKdLdKdQdKjHdKdYdKd*dLjVdLjQdLjOdLdSdLdRdLd@"
		"dLjHdLdZdMeadMe3dMe2dMe1dMdVdMdWdMd%dMd^dMd&"
		"dNe4dNe2dNdOdNdWdNdYdNdPdNdVdNd^dNd%dNd*dNdQdOd*dOd!"
		"dOdRdOdPdOdXdOdQdOdWdOdZdOd&dPe7dPe3dPdQdPdYdPdZdPdRdPd!dPd&"
		"dQe7dQe6dQdXdQdRdQdZdQd!dQdSdQe0dQdYdRdXdRdSdRdZdRd@dRe0dRd*dSjHdSd@"
		"dSd!dSjYdSjIdSjXdSjVdSe0dTd#dTd$dTd%dTe1dTd^dUdVdUd$dUd%dUdWdUd^"
		"dUe1dUdXdUd&dUecdUdYdVeadVe3dVe2dVdYdVdWdVd%dVd^dVdXdVe1dVd#dVd&"
		"dWe4dWdXdWd^dWd%dWd&dWdYdWe2dWe1dXdYdXd*dXe4dXe0dYe7dYd&dYd*"
		"dYe0dYe3dZehdZegdZd!dZe0dZd*dZe6dZe7d!e0d!d*d!e8d!jRd@egd@jId@jUd@jRd@"
		"e7d@jTd@jXd@jYd#d$d#d%d#d^d$d%d$e1d$d^d$ecd$d&d$eid%e4d%d^d%e1d%d&d%"
		"ead%ecd^d&d^e2d^e1d^e3d^ecd&e3d&e4d&ecd*e0d*e5d*efd*eed*"
		"eme0e6e0e7e0ege0e8e0efe1ece1ede1eje1eMe2e3e2ece2ede2e4e2eke2ele2e5e3ej"
		"e3e4e3e5e3ebe3efe3e6e4e5e4ede4e6e4ege4ehe5e6e5efe5eee5ege5eme5ehe5eke5"
		"eVe6ege6efe6eee6eoe6ele6eFe7ege7efe7ene7epe7jIe7eTe8jTe8jUe8jIe8ege8jC"
		"e8jXe8efe8jYe8jWe8kye9eae9eie9eje9eLe9ebe9eBeaebeaeceaedeaekeaeeebeceb"
		"ejebedebeBebeMebekebeeebesecedeceeecejeceBedeeedeledefedeMedeNedfjedfk"
		"eeefeeekeeemeeeseeeVeeeMeeeNefeFefemefelefeVeffCegehegeFegepegjUeheoeh"
		"enehepeheTehemeheVehkyehkxeieLeiejejeBejeMekeBekenekelekesekeMekeNekfj"
		"ekfkekemekeVekeUeleBeleoelemeleseleUelfCelfBeleneleMeleNelfjemepemeVem"
		"enemfCemeFemesemeoeneoeneFeneVenepeneTenesenkyeoepeoeFeoeTeokyeoeVeokx"
		"eokvepeTepeFepjTepjSepjUepjCeqf6eqf5eqe&eqe^"
		"eqf1eqf2eqeKeqePeqeJeqeOerk%erk$erk@"
		"erkeerfgerfferf8erf7erfberfcereWere!ereXere#"
		"erkderkcesfBesfzesfmesfkeseuesfjesfCeseMeseVeseNeseUetgketgjetghetg9et"
		"fYetfOetfMetfxetfveteZetewetgdetggetgeetfwetfNeteReteYeteQeufGeufFeufD"
		"eufqeufoeueVeueUeueNeueMeufkeufBeufjeufCeufleufAeufnevfJevfHevfGevfEev"
		"feevfdevf0eve*"
		"evfoevfrevfFevfKevfnevfpevfsevfuewfKewfJewfIewftewfreweZewfvewfOewfwew"
		"fNewfyewfLewfsexe&exe$exe^"
		"exfmexeJexeNexfjexf2eyf7eyf8eyfaeyfzeyeUeyeXeyfCeyfbezf5ezf4ezf6ezfxez"
		"eOezeQezfwezf1eAfgeAffeAfheAfMeAeZeAe#"
		"eAfNeAfceBeLeBeMeCeKeCeLeDePeDeSeEgfeEfXeEg9eEgceEgaeEeReEeSeEg5eEg6eE"
		"g8eFeTeFeVeGkxeGkbeGkweGkyeGeTeGeWeGkceGkBeHl5eHl3eHl1eHk$eHk%eHk#eHe!"
		"eHe@eHl0eHk*eHk^";

	const char coded_edges2[] =
		"eIl5eIgheIgkeIgieIeYeIe@eIgoeIgneIgleJe&eJeKeJf2eJe^eJe*eJe%"
		"eKf2eMeNeMfjeNe&eNfjeNfmeNe$"
		"eOf5eOePeOf1eOf6eOf0eOf3eOf2ePf1eQf5eQeReQfweQfxeQf4eQggeQgdeQgfeRgceR"
		"gaeRfXeRfweRggeRgdeRgfeRg9eSg9eSg8eSg5eSgceSgbeTkCeTkAeTkbeTkyeTkxeTkv"
		"eTkBeUf7eUeVeUfCeUfzeUfaeVfCeWkyeWkxeWkweWeXeWfbeWkceWkdeWkbeWfceXf7eX"
		"fbeXf8eXkceXkdeXfeeXf9eYgneYggeYgeeYfZeYfYeYeZeYfNeYgkeYgjeYgheYgdeZgg"
		"eZfYeZfgeZfNeZfMeZfheZgdeZgee!kde!kce!e#e!fce!k%e!k@e!k$e!kee@l6e@l2e@"
		"k%e@k#e@gke@gie@ghe@fPe@gle@goe@k*e@l0e@l5e@k$e#kce#fge#fce#ffe#kde#"
		"fde#fie#fbe$fme$fle$e%e$e&e$fqe$fne%fme%fqe%fne%fpe%e*e%fle%fke%e^e^"
		"fpe^f2e^e&e^e*e^fqe&fme&f2e*fue*fre*fpe*foe*fqe*f0e*"
		"f2f0fpf0fof0fuf0frf0ftf0f3f0f1f0f6f1fuf1f5f1f6f1f2f2fpf3fxf3ftf3fsf3fu"
		"f3fyf3fvf3f4f3f6f4fxf4fyf4f5f4ftf4fsf5f6f5fxf6fuf6ftf7f8f7faf7fzf7fbf8"
		"fEf8fbf8f9f8fef8fDf9fzf9fDf9fEf9fGf9fef9fAf9fBf9fafafzfafAfafDfafGfbk%"
		"fbfEfbkcfbfcfbfefbkdfckcfcfHfcfgfcfffcfdfckefckdfdfFfdfEfdfHfdfIfdfKfd"
		"fifdfefdfffefKfefHfefEfefDfefFfffHfffgfffifffIfgfhfgfMfhfMfhfLfhfifhfI"
		"fhfJfifMfifIfifHfifJfifLfifOfjfkfjfmfkfofkflfkfnfkfqflfmflfqflfnflfpfm"
		"fqfnfofnfqfofFfofpfofrfpfqfrfKfrfsfrfufrfvfsftfsfvfsfyftfuftfyftfvfufy"
		"fvfwfvfyfvggfvgdfvgffwfxfwggfwgdfwgffxfyfxgdfzfAfzfCfzfDfAfBfAfDfAfGfA"
		"fEfBfFfBfCfBfGfBfDfDfEfDfGfEfFfFfGfHfLfHfIfHfKfIfJfIfLfIfOfJfKfJfOfJfL"
		"fKfOfLfMfLfOfMgefMfNfMgdfNggfNfYfNfOfNgdfNgefOgefOgdfPl8fPl7fPk&"
		"fPfRfPgnfPgmfPgofPl6fPfQfPgifPfZfPl5fPghfPgjfPf#fQl9fQl7fQfRfQl6fQf#"
		"fQf^fQgnfQgmfQfSfQgofQfZfQl8fQl5fRlbfRlafRl7fRg!fRgAfRgufRfSfRf^fRf#"
		"fRl6fRl9fRl8fRg0fRgsfRgpfRgrfSlbfSlafSl8fSg!fSgufSg1fSf*"
		"fSgsfSgpfSgrfSl9fSg0fSgtfSgxfSgwfSgyfSf^"
		"fSgAfTgEfTfVfTg6fTg7fTg5fTgbfTgafTgcfTfUfTfXfTf!fUgEfUfVfUf!fUf$"
		"fUg6fUfWfUg7fUfXfUg5fVfWfVf$fVgBfVgEfVf!fVgCfVf&fWgNfWgHfWg4fWg3fWf*"
		"fWgBfWgCfWgEfWf&fWgMfWgIfWgFfWgLfWf$fWgJfXg7fXg5fXf@"
		"fXgafXgffXg9fXgbfXggfXgefXfYfXf!fXg6fYgifYg9fYf#fYf!"
		"fYgefYgffYgdfYfZfYf@fYgjfYgkfZl6fZf@fZgjfZgifZgkfZf#"
		"fZgnfZgofZgefZgdfZgmf!gEf!g6f!f@f!f$f!f%f!gff!gef!gaf!gbf@f*f@f#f@f%f@"
		"gef@gff@f$f@f^f@gdf@gjf#l6f#gmf#f^f#f%f#gjf#gif#gkf#gnf$gGf$gEf$f%f$f&"
		"f$g4f$f*f$gJf$gMf$gBf%gKf%gEf%gzf%gvf%f^f%f*f%g1f%g4f%f&f%g0f%gAf%gJf^"
		"g!f^gvf^gpf^g0f^g1f^f*f^gxf^gAf^gsf&gNf&g1f&gJf&gMf&g4"
		"f&f*f&gFf&gBf&gCf&gIf&gGf&g3f*g1f*g4f*g0f*gxf*"
		"gMg0l9g0gZg0gpg0g4g0gxg0gAg0g1g0gwg0gtg0gvg0gsg0grg0g2g1gpg1gAg1gzg1gx"
		"g1g2g1g4g1gsg2gXg2gWg2gVg2gUg2gTg2gPg2gZg2gzg2gyg2gAg2g3g2gvg2gwg2gug2"
		"g!g2gYg2grg2gsg2gqg3g!"
		"g3gYg3gXg3gWg3gUg3gTg3gSg3gRg3gQg3gOg3gKg3gLg3gJg3g4g3gZg3gGg3gFg3gHg3"
		"gNg3gPg3gCg3gBg4gJg4gKg4gMg4gCg5g6g5g8g5gbg5gcg5gag6g7g6gbg6gcg6gag7g8"
		"g8gbg8gcg8gag9gag9gcg9ggg9gfg9gdgagbgagggagfgbgcgbgggbgfgbgdgdgkgdgjgd"
		"gegdgggegkgegigegfgegjgfgggggkghgighgkghgoghgnghglghl5ghk*"
		"gigjgigogigngiglgjgkgjgogjgngjglgll7gll2glk%glgmglgogll5glk*gll0glk&"
		"gll6gml8gml7gml2gmgngml5gml6gmk*gml0gmk&gmk$gngognl6gok*gol0gok&"
		"gplbgpgqgpgsgpl9gpgxgpgAgplagpl8gqlbgqgYgqgrgqgtgqgugqgwgqg!"
		"gql9gqgygqgzgqlagql8grgYgrgsgrgtgrgugrgwgrg!"
		"grgygrgxgrgzgrlagslbgsl8gsgZgsgYgsgtgsgwgsgugsl9gsgxgsgygsgAgtlbgtgVgt"
		"gugtgwgtg!gtgygtgxgtgzgtlagugWgugVgugvgug!"
		"gugZgugYgulagvgXgvgWgvgVgvgUgvgwgvg!"
		"gvgygvgxgvgzgvgZgvgYgwlagwgYgwgygwgxgwgzgwgZgxl9gxgYgxgygxgAgylagyl9gy"
		"gYgygzgygZgyg!gzgYgzgAgzgZgzg!"
		"gAgZgBgNgBgCgBgEgBgIgBgFgBgHgBgMgBgLgBgJgCgOgCgNgCgDgCgIgCgFgCgHgCgMgC"
		"gLgCgJgDgOgDgEgDgIgDgFgDgHgDgMgDgLgDgJgDgNgFgQgFgPgFgGgFgIgFgLgFgMgFgK"
		"gGgTgGgSgGgRgGgQgGgHgGgNgGgLgGgMgGgKgGgOgGgPgGgZgHgSgHgRgHgQgHgPgHgIgH"
		"gNgHgOgIgRgIgOgIgNgIgLgIgMgIgKgJgKgJgMgKgLgKgNgKgPgKgOgLgPgLgOgLgMgLgN"
		"gMgOgMgNgNgTgNgSgNgRgNgQgNgOgNgPgOgUgOgSgOgPgOgQgOgRgOgTgOgXgPgWgPgVgP"
		"gUgPgSgPgQgPgXgPgTgPgZgPgRgPgYgQgVgQgRgQgTgQgSgQgXgQgUgQgZgQgYgQgWgRgV"
		"gRgSgRgTgRgXgRgUgRgWgRgZgRgYgSgTgSgUgSgXgSgWgSgVgSgYgSgZgTgXgTgUgTgWgT"
		"gYgTgVgTgZgUgWgUgVgUgXgUgYgUgZgUg!gVgWgVgXgVgYgVg!gVgZgWgXgWgYgWgZgWg!"
		"gXgYgXgZgXg!gYgZgYg!gZg!g!l9g@g#g@g$g@g%g@h5g@g^g@h3g@g*g@h4g@g&g@h6g@"
		"hCg@hzg@h0g@h2g@hBg#g$g#g%g#g^g#g*g#h3g#h5g#g&g#hCg#hzg#h4g#hBg#hvg#"
		"hyg#h0g$lrg$h6g$g%g$h3g$g*g$h5g$g^g$h4g$g&g$h0g$h2g$hzg$hCg$hAg$hGg%"
		"h5g%h4g%g^g%g*g%g&g%h3g%hzg%hCg%hAg%hvg%hGg%hyg%h0g%hDg^h5g^h3g^g&g^g*"
		"g^hCg^hzg^hBg^hvg^hyg^hwg^hGg^hfg^hbg^hDg&h5g&h3g&hzg&hCg&hAg&hGg&g*g&"
		"hvg&hFg&hfg&hDg&hwg&hyg&hgg*h6g*h5g*h4g*h3g*h0g*hGg*hDg*hzg*hFg*hCg*"
		"hAg*h2g*hKg*"
		"hfh0lrh0h6h0h5h0h4h0h2h0hKh0hHh0hJh0h1h0h3h0hDh0hhh0hGh0hEh0hOh0hLh0hN"
		"h0hgh1lrh1h6h1h4h1h3h1lnh1hLh1hOh1hHh1hIh1lAh1hKh1h2h1h7h1lyh1hhh1lzh1"
		"lBh1lmh2lyh2lrh2h6h2h5h2h3h2lnh2h4h2hHh2hKh2hIh2hOh2hLh2lzh2lmh2lqh2hN"
		"h3lrh3h6h3h4h3h5h3hKh3hHh3lqh3hDh3hJh4h5h4h6h4lqh4lrh4lmh4lnh4lzh4lph5"
		"h6h5lrh5lqh5lmh6lrh6lqh6loh6lmh6lzh6lnh7lDh7lCh7hNh7hMh7hOh7lBh7h8h7hI"
		"h7hhh7lAh7hHh7hJh7hkh7h9h8lEh8lCh8hHh8h9h8lBh8hkh8hnh8hNh8hMh8hah8hOh8"
		"hhh8lDh8lAh9lCh9lAh9h!"
		"h9hNh9hMh9hah9hnh9hkh9lBh9lEh9lDh9hqh9hSh9hPh9hRhalGhalFhalDhaiihahUha"
		"hrhahphahShahPhahRhalEhahqhahThahXhahWhahYhahnhah!"
		"hbhGhbhFhbhihbhdhbhwhbhxhbhvhbhBhbhAhbhChbhchbhfhchChchvhchdhchihchlhc"
		"hwhchehchxhchfhdhxhdhehdhlhdh@hdh%hdhihdh#hdhoheh*hehuhehthehpheh@heh#"
		"heh%hehohei4hei0heh^"
		"hei3hehlhei1hfhxhfhjhfhAhfhFhfhzhfhBhfhGhfhEhfhghfhihfhwhfhvhghzhghEhg"
		"hFhghDhghhhghjhghJhghKhghIhghihghkhghAhhlBhhlAhhhjhhhJhhhIhhhKhhhkhhhN"
		"hhhOhhhEhhhDhhhMhhhFhihGhihzhihxhihjhihlhihmhihFhihEhihAhihBhjhKhjhzhj"
		"hphjhkhjhmhjhEhjhFhjhlhjhnhjhDhjhJhklBhkhOhkhMhkhEhkhnhkhmhkhJhkhIhkhK"
		"hkhNhlh&hlh%hlhmhlhohlhuhlhphli1hli4hlh@"
		"hmi2hmhZhmhnhmhphmhrhmhuhmhohmhqhmh!hmi1hnhqhnhrhnhphnhXhnh!"
		"hnhShohrhoi1hoi4hohuhohphoh^hoh@hoh#hoi0hoh&"
		"hohthphrhphuhphqhphXhpi4hqlEhqihhqhPhqhuhqhXhqh!"
		"hqhrhqhWhqhThqhVhqhShqhRhqhshrhPhrh!"
		"hrhZhrhXhrhshrhuhrhShsifhsi7hsihhshZhshYhsh!"
		"hshthshVhshWhshUhsiihsighshRhshShshQhtifhti2hti3hti1hthuhtihhth&hth^"
		"hth*hti5hti7hth#hth@hui1hui2hui4huh#"
		"hvhwhvhyhvhBhvhChvhAhwhxhwhBhwhChwhAhxhyhyhBhyhChyhAhzhAhzhChzhGhzhFhz"
		"hDhAhDhAhBhAhGhAhFhBhChBhGhBhFhBhDhDhEhDhGhDhJhDhKhEhFhEhJhEhKhEhIhFhG"
		"hGhKhHlBhHhIhHhKhHhOhHhNhHhLhHlAhHlnhIlBhIhJhIhOhIhNhIhLhIlAhJlBhJlAhJ"
		"hKhJhOhJhNhJhLhLlzhLhMhLhOhLlAhLlBhLlnhLlyhLlChMlDhMhNhMlAhMlBhMlnhMlC"
		"hMlyhNlChNhOhNlBhOlyhOlnhPlQhPlGhPhQhPhShPlEhPlThPhXhPh!"
		"hPlFhPlDhQlGhQlDhQighQhRhQhThQhUhQhWhQiihQlEhQlThQhYhQhZhQlFhQlQhRlFhR"
		"ighRhShRhThRhUhRhWhRiihRhYhRhXhRlThRhZhSlGhSlDhSihhShThShWhShUhSlEhShX"
		"hShYhSh!"
		"hTlShTlFhThUhThWhTiihThYhThXhThZhTlThUlXhUlFhUhVhUiihUlThUihhUighUlShV"
		"hWhViihVhYhVhXhVhZhVlThVihhVighVlShWlFhWighWhYhWhXhWhZhWihhXhYhXh!"
		"hYighYhZhYihhYiihZighZh!hZihhZiih!ihh@h#h@h%h@i0h@h^h@h*h@i4h@i3h@i1h#"
		"i1h#h$h#i0h#h^h#h*h#i4h#i3h$h%h$i0h$h^h$h*h$i4h$i3h$i1h^i7h^h&h^i0h^"
		"i3h^i4h^i2h&ihh&h*h&i5h&i3h&i4h&i2h&i6h&i7h*i7h*i6h*i0h*"
		"i5i0i6i0i5i0i3i0i4i0i2i1i2i1i4i2i3i2i5i3i4i3i5i5i8i5i6i5i7i6ibi6i7i6i8"
		"i6i9i7i8i7ifi7ibi7ihi7i9i8i9i8ibi8iai8ifi8iMi8iJi9ifi9iai9ibi9iMi9iJi9"
		"iLi9ijiaiMiaiJiaiLiaibiaijiairiaiciainiaifibifibicibiJibiMibiKibieibir"
		"ibijiclWicieicidicificiUiciRiciTicm1icm0icmticiricmsicmqiciJidlXidifid"
		"m0idm1idmtidmqidmsidmxidiRidmwidiUidieidmuidiSidlWidmaiem1ieifielWieig"
		"iem0ielXiemtielSiemqiemxieiRieiUifigifihiflSifiiifm1iglWiglRigihigiiig"
		"lSiglTiglXihiiiilTiilSiilFiilEijirijiLijiKijiMijinijikijitijiwikiLikiK"
		"ikiJikinikilikitikiuikirikiwikiPikiQikiOiliYiliXilixiliAiliyiliPiliOil"
		"iQilimimncimm#"
		"imj8imj5imiyimiximiXimiWimiYimmYimmZimmXimisiniMiniLiniKiniwinitinivin"
		"iBiniEioiIioiGioiFioiuioiAioivioizioitioixioiBioiCipi@"
		"ipivipiuipiCipiDipiIipiBipiHipiFipi%"
		"iqmtiqm7iqiYiqiNiqiTiqiOiqiSiqiQiqiUiqisiqmsiriTiriQiriKiriNiriPiriJir"
		"iLisiPisiNisiVisiYism8ismZismWism7ism9ismYitiMitiuitiwitiLitiKiuiviuiB"
		"iuiEiuiCiviwiviBiviEiviCiwiBiwiEiwiCixiyixiAixj8ixiXixiYiyj8iyi#"
		"iyiziyiFiyiGiyiIiyi@izi#iziAiziFiziGiziIizi@izj8iAiFiAiGiAiIiAi@"
		"iBiCiBiEiCi%iCiIiCiHiCiDiDi%iDiEiDiIiDiFiDiHiFiGiFiIiFi@iGiHiGi@iGi%"
		"iHiIiHi@iHi%iIi%"
		"iJiKiJiMiJiQiKiLiKiQiLiMiNiOiNiQiNiTiNiSiNiUiOiPiOiTiOiSiOiUiPiQiPiYiP"
		"iViQiTiQiUiRm0iRiSiRiUiRmsiRmtiRmriRm6iRmxiSiTiSmsiSmtiSmriSm6iTiUiTm6"
		"iUmxiUmsiUmtiUmriVm7iViWiViYiVmZiVmYiVmWiVm9iVm8iWiXiWmZiWmYiWmWiWm9iW"
		"m8iXiYiXmZiXmYiXmWiXm9iYm8iYm7iZi^iZi#iZj7iZj8iZi&iZj0iZi!iZi@iZj4iZi$"
		"i!i#i!j7i!j4i!i&i!j1i!j6i!i^i!j9i!n0i!npi@j8i@i^i@i#i@i%i#j0i#i^i#i$i#"
		"j7i#j8i$j0i$i%i$i*i$i^i^j9i^i&i^j7i^j0i^j4i^j8i&j4i&j9i&i*i&j7i&j3i&"
		"jci&j1i*j0i*jci*j9i*"
		"j4j0jcj0j8j1ncj1j7j1npj1ndj1j2j1nJj1n0j1j6j1nsj1j4j1nKj2jaj2nsj2ndj2jr"
		"j2oKj2juj2npj2j3j2oAj2oBj3jaj3j9j3j4j3juj3jfj3jrj3jdj3nsj4j9j4jaj4j7j5"
		"mZj5nlj5ncj5j8j5m#j5m$j5m@"
		"j5nkj5j6j5mYj6ndj6nkj6njj6n0j6nKj6j7j6ncj6nJj6npj6noj7j9j7j8j7n0j8nlj9"
		"jaj9juj9jcjajfjajujajdjajgjajbjajijbjcjbjijbjfjbjhjcjijdjqjdjjjdjgjdjt"
		"jdjujdjkjdjmjdjejdjfjejtjejqjejkjejnjejsjejjjejvjeopjeoOjfjgjfjijfjjjf"
		"jujfjtjgjmjgjjjgjhjgjtjgjujhjljhjjjhjmjhjijjjvjjjkjjjtjjjmjjjqjjjujkjq"
		"jkjvjkjljkjtjkjpjkjyjkjnjljqjljmjljyjljvjnjtjnoOjnoCjnjojnp0jnopjnjsjn"
		"oRjnjqjnp1joq8jojwjooRjooCjoq7joq4joq6joqbjooOjoqajojpjpjwjpjvjpjGjpjq"
		"jpjJjpjMjpjKjpjzjqjvjqjwjqjtjroKjroBjrnsjrjujroJjrjsjrndjrnejsoCjsoJjs"
		"oIjsopjsp1jsjtjsoBjsp0jsoOjsoNjtjujtopjuoKjunsjvjwjvjGjvjJjvjyjwjGjwjJ"
		"jwjMjwjNjwjOjwjQjxjyjxjQjxjNjxjPjzoCjzjKjzjJjzk6jzk3jzjGjzk5jzq6jzq7jz"
		"q5jzjAjAk6jAk5jAjGjAj$jAj&jAjKjAjLjAjJjAjDjAjBjBjZjBj^jBj&"
		"jBjWjBjVjBjEjBjXjBjCjCkCjCkwjCkvjCjWjCjSjCjRjCj#jCjTjCj@"
		"jCjIjCjZjCkzjDqDjDpUjDpTjDk0jDk5jDk4jDk6jDj$jDj*jDj%jDk2jDj&"
		"jEkKjEkJjEkHjEkGjEkEjEj#jEj^jEj%jEj&jEk1jEk2jEjZjEj!"
		"jFqMjFqKjFqHjFqGjFqEjFkNjFkLjFkKjFkIjFk0jFqCjFj*"
		"jFqDjFk1jFqBjFkHjFkOjGjJjGjKjGjMjHjMjHjLjHjKjHjOjHjPjHjVjHjYjIjRjIjUjI"
		"jSjIjXjIjWjIjYjJjNjJjKjJjMjJjOjKjLjKj&jLjMjLjOjLjNjLjPjLj&"
		"jMjPjMjOjMjNjNjOjNjQjOjPjPjQjRjSjRjUjRjXjRjWjRjYjSkCjSkBjSkzjSkvjSjTjS"
		"jXjSjWjSjYjTkyjTkxjTkvjTjUjTkCjTkzjUjYjUjXjUjWjVjWjVjYjVj#jVj&"
		"jWjXjXjYjZj^jZj!jZj#jZkGjZkDjZkFj!kfj!j@j!kGj!kDj!kFj!kzj!kAj!kCj@kCj@"
		"kfj@j#j@kGj@kzj@kDj@kFj@kAj#kzj#kAj#kCj#kfj$j%j$j&j$k2j$j*j$k1j$k5j%j^"
		"j%k2j%j*j%k1j%k5j^j&j^k2j^k1j^j*j&k5j&k6j*qCj*k0j*k2j*qDk0kIk0k1k0kH"
		"k0kKk0qCk0qDk1qCk1kHk1k2k1kKk2qDk2kKk3q7k3k4k3k6k3pTk3pUk3pSk3q6k3q5k4"
		"qDk4k5k4pTk4pUk4pSk4q6k4pVk5k6k5pUk6q7k6pSk6q6k6q5k7qPk7qOk7qFk7kNk7kM"
		"k7kOk7qNk7k8k7kIk7khk7qMk7kHk7kJk7kkk7k9k8qQk8qOk8k9k8qNk8kkk8knk8kNk8"
		"kMk8kak8kOk8khk8qPk8qMk9qOk9k!"
		"k9kNk9kak9knk9kkk9qNk9qQk9qPk9kqk9kSk9kPk9kRkaqSkaqRkaqPkalikakUkakrka"
		"kpkakSkakPkakRkaqQkakqkakTkakXkakWkakYkaknkak!"
		"kbkwkbkxkbkvkbkBkbkAkbkCkbkckbkfkbkikck%"
		"kckxkckwkckdkckikcklkdkikdkekdklkdk@kdk%kel1kek*kekukektkekpkeklkek@"
		"kek#kek%kekokel4kel0kek^"
		"kel3kfkxkfkvkfkjkfkAkfkFkfkzkfkBkfkGkfkEkfkgkfkikfkwkgkEkgkFkgkDkgkhkg"
		"kjkgkJkgkKkgkIkgkikgkkkgkAkhqNkhkjkhkJkhkIkhkKkhkkkhkNkhkOkhkEkhkDkhkM"
		"khkFkikGkikxkikwkikjkiklkikmkikFkikEkikAkikBkjkpkjkkkjkmkjkEkjkFkjklkj"
		"knkjkDkjkJkkqNkkkMkkkEkkknkkkmkkkJkkkIkkkKkkkNklk%"
		"klkmklkoklkuklkpkll1kll4klk@kml2kmkZkmknkmkpkmkrkmkukmkokmkqkmk!"
		"kml1knkqknkrknkpknkXknk!knkSkokrkol1kol4kokukokpkok^kok@kok#kol0kok&"
		"koktkpkrkpkukpkqkpkXkpl4kqqQkqlhkqkPkqkukqkXkqk!"
		"kqkrkqkWkqkTkqkVkqkSkqkRkqkskrkPkrk!"
		"krkZkrkXkrkskrkukrkSkslfkslhkskZkskYksk!"
		"ksktkskVkskWkskUkslikslgkskRkskSkskQktlfktl2ktl3ktl1ktkuktlhktk&ktk^"
		"ktk*ktl5ktl7ktk#ktk@kul1kul2kul4kuk#"
		"kvkwkvkykvkBkvkCkvkAkwkxkwkBkwkCkwkAkxkykykBkykCkykAkzkAkzkCkzkGkzkFkz"
		"kDkAkFkAkDkAkBkAkGkBkDkBkCkBkGkBkFkDkEkDkGkDkJkDkKkEkFkEkJkEkKkEkIkFkG"
		"kGkKkHkIkHkKkHkOkHkNkHkLkHqMkHqGkIqNkIkJkIkOkIkNkIkLkJkKkJkOkJkNkJkLkL"
		"qOkLqJkLkMkLkOkLqMkLqGkLqHkLqFkLqNkLqCkMqPkMqOkMqJkMkNkMqMkMqNkMqGkMqH"
		"kMqFkMqCkNkOkNqNkOqGkOqHkOqFkPq#kPqSkPkQkPkSkPqQkPq^kPkXkPk!"
		"kPqRkPqPkQqSkQqPkQlgkQkRkQkTkQkUkQkWkQlikQqQkQq^kQkYkQkZkQqRkQq#"
		"kRqRkRlgkRkSkRkTkRkUkRkWkRlikRkYkRkXkRq^"
		"kRkZkSqSkSqPkSlhkSkTkSkWkSkUkSqQkSkXkSkYkSk!kTq%"
		"kTqRkTkUkTkWkTlikTkYkTkXkTkZkTq^kUr1kUqRkUkVkUlikUq^kUlhkUlgkUq%"
		"kVkWkVlikVkYkVkXkVkZkVq^kVlhkVlgkVq%kWqRkWlgkWkYkWkXkWkZkWlhkXkYkXk!"
		"kYlgkYkZkYlhkYlikZlgkZk!kZlhkZlik!lhk@l1k@k#k@k%k@l0k@k^k@k*k@l4k@l3k#"
		"k$k#l0k#k^k#k*k#l4k#l3k#l1k$k%k$l0k$k^k$k*k$l4k$l3k$l1k^k&k^l0k^l3k^"
		"l4k^l2k&lhk&l7k&l6k&k*k&l5k&l3k&l4k&l2k*l7k*l6k*l0k*"
		"l5l0l6l0l5l0l3l0l4l0l2l1l2l1l4l2l3l2l5l3l4l3l5l4l5l5l8l5l6l5l7l6lbl6l9"
		"l6l7l6l8l7lel7ldl7lcl7lal7l8l7lfl7lbl7lhl7l9l8ldl8lcl8l9l8lbl8lal8lfl9"
		"lfl9lcl9lal9lblaldlalblalclalflbrclbldlblflblclblelblglclelcldlclflcrd"
		"lcrclclglcr0lcr1lcq%lclhldrcldrdldleldr0ldlfldr1ldlgldq%ldlildq^"
		"lerdlelfler0lelglercler1leq%leq$lelhleq*lelilfrclflglflhlfq%"
		"lflilfrdlgrclgr0lgq$lglhlglilgq%lgq^lgr1lhliliq^liq%"
		"liqRliqQljrWljlvljlpljloljrnljreljrwljlkljroljrfljrxljlsljrGljrplklmlk"
		"lplklolklslklvlkrwlkrFlkrnlkrelkrxllsullsellrWlllMlllLlllullscllsllllx"
		"lllNlllwllsdllsmllltllr&"
		"llstlmlulmlzlmlvlmlplmlqlmlnlmlwlnlMlnlvlnlulnlylnlzlnlAlolplolrlolslo"
		"rnlorelorwlorFlplvlplqlplslplzlprwlprFlqlrlqlvlqlulqlzlssdlsltlsrFlsrW"
		"lsr^lsrGlsrnlsrylslvltlNltrWltr^"
		"ltrFltlwltlxlts4ltrHltlululvlulzlulwlur^lusclvlzlvrFlwlxlwr^"
		"lwsclwlNlwrWlwsllwlzlxstlxsmlxrWlxlLlxlMlxlNlxsclxsllxr^"
		"lxlylxsdlylClylzlylAlylMlzlAlAlMlAlDlAlBlAlClBlMlBlKlBlGlBlClBlDlBlElC"
		"lDlClKlClGlClMlClElClLlDlHlDlElDlGlDlFlDlKlElKlElFlElGlElTlElQlFlKlFlQ"
		"lFlTlFlGlFlHlFlOlGlRlGlIlGlKlGlHlGlQlGlJlGlTlGl^lHlJlHlIlHl^lHlKlHl#"
		"lHsxlHlQlHsMlHlLlHlOlHswlHsylIl#lIsMlIsxlIlJlIsylIl^"
		"lIsJlIsLlIswlIszlIl$lIlKlIsulJsxlJlKlJlLlJswlJsulJl#lJsMlJl^"
		"lJsylJsvlJszlJsJlKlLlKlMlKsulKsxlKlNlKswlLsulLlMlLlNlLstlLsllLswlLsxlL"
		"svlLsclMlNlMsllMsulMstlMsclMsmlNsllNstlNsclNsulNselNsvlNr*"
		"lNsnlNswlOlUlOlRlOl%lOl^lOlVlOlXlOlPlOlQlOl@lOlSlPsQlPsPlPl%lPl@"
		"lPlVlPlYlPl$lPlUlPl&lPs#lPlRlQlRlQlTlQlUlQl^lRlXlRlUlRlSlRl%lRl^"
		"lSlXlSlTlSlWlSlUlUl&lUlVlUl%lUlXlUl@lUl^lVl@lVl&lVlWlVl%lVl!"
		"lVm1lVlYlWlXlWm1lWl&lWl@lXm1lYsQlYsLlYlZlYs#lYs@lYl$lYsPlYl@lYl%"
		"lYmJlYmGlZtqlZtnlZtllZtjlZsPlZmJlZmGlZs@lZmClZmDlZmIlZtmlZmFlZs#l!mul!"
		"mcl!l*l!l&l!mFl!myl!mEl!mCl!l@l!mBl!mzl!mbl@l$l@l&l@l*l@l%l@myl@mzl#"
		"sMl#l^l#sLl#l$l#syl#sQl#sxl$l%l$sLl$sQl$sPl$sMl$sKl%l^l%l&l^sxl^sMl&l*"
		"l&mcl&m1l*mxl*mul*myl*mBl*mzl*mbl*mFl*mEl*"
		"mCm0mBm0m1m0mxm0mum0mwm0mqm0mtm0mrm0m6m1mqm1mtm1mrm2t0m2mfm2m4m2mIm2mH"
		"m2mJm2tlm2tmm2tkm2m3m2s*m2mDm2mcm2mCm2mEm3tlm3m4m3t0m3mfm3t1m3mim3s*"
		"m3mIm3mHm3m5m3mJm3mcm4m5m4t1m4mim4t0m4mfm4tSm4t2m4tPm4tRm4mlm5t1m5mVm5"
		"mTm5mPm5mkm5mim5mNm5tSm5mKm5tRm5mMm5tPm5t2m5mlm5mOm5mSm5mRm6mBm6mAm6m8"
		"m6mrm6msm6mqm6mwm6mvm6mxm6m7m6mam6mdm7mZm7mxm7mqm7m8m7mdm7mgm7mrm7m9m7"
		"msm7mam8msm8m9m8mgm8mWm8mZm8mdm8mXm9m%m9m#m9mkm9mgm9mWm9mXm9mZm9mjm9m*"
		"m9m$m9m!m9m&"
		"mamemamvmamAmamumamwmamBmamzmambmamdmamrmamqmamsmbmvmbmumbmfmbmzmbmAmb"
		"mymbmcmbmembmEmbmFmbmDmbmdmcmAmcmemcmEmcmDmcmFmcmfmcmImcmJmcmzmcmymcmH"
		"mdmBmdmumdmsmdmqmdmemdmgmdmhmdmAmdmzmdmvmdmwmemkmemfmemhmemzmemAmemgme"
		"mimemymemEmfmimfmhmfmEmfmDmfmFmfmImgmZmgmhmgmjmgmpmgmkmgm%mgm*mgmWmhm^"
		"mhmimhmkmhmmmhmpmhmjmhmlmhmVmhm%mimlmimmmimkmimSmimVmimNmjm%mjm*"
		"mjmpmjmkmjm!mjmWmjmXmjm$mjm@mjmomkmmmkmpmkmlmkmSmkm*"
		"mlmKmlmSmlmVmlmmmlmRmlmOmlmQmlmNmlmMmlmnmmmVmmmUmmmSmmmnmmmpmmnvmmnumm"
		"ocmnnnmnnfmnmQmnnvmnnumnocmnobmnn2mnmUmnmTmnmVmonumonbmom#mom@"
		"mompmonmmonnmon@mon#mon2mom^mom&mom%mpm%mpm^mpm*mpnmmpnnmpn@"
		"mqmAmqmrmqmtmqmwmqmxmqmvmrmsmrmwmrmxmrmvmsmtmtmwmtmxmtmvmumvmumxmumBmu"
		"mAmumymvmwmvmBmvmAmvmymwmxmwmBmwmAmwmymymzmymBmymEmymFmzmAmzmEmzmFmzmD"
		"mAmBmBmFmCmDmCmFmCmJmCmImCmGmCtmmCtlmCtjmDtjmDmEmDmJmDmImDmGmDtmmEmFmE"
		"mJmEmImEmGmFs@mGs#mGmHmGmJmGtmmGtlmGs*mGtpmGtqmGs@mGtomHs#"
		"mHmImHtlmHtmmHtkmHs*mHs@mHt0mHt3mImJmIs*mIt0mJtomJs#mJtmmJtpmJtqmJs@"
		"mKmVmKmLmKmNmKtRmKtQmKtSmKtWmKtVmKtTmKuemKmSmLu5mLtYmLtTmLngmLmUmLmTmL"
		"mMmLmOmLtRmLmPmLtQmLmRmLtSmLtWmLntmLtVmLuemMu5mMmUmMmNmMmOmMmPmMmRmMnt"
		"mMtWmMuemMtVmMmTmMmSmNmVmNmOmNtRmNmRmNmPmNtSmNtQmNmSmNmTmOmPmOmRmOntmO"
		"mTmOmSmOmUmOuemPmQmPnfmPntmPuemPnvmPtVmPtWmQmRmQnfmQmTmQntmQmSmQmUmQnv"
		"mQuemQtVmRtSmRnfmRmTmRmSmRmUmRnvmSmTmSmVmSnfmTmUmTnvmTnfmUmVmUnvmUnfmU"
		"ocmVnvmVocmWm&mWm%mWmXmWmZmWm$mWm!mWm#mWm*mXnlmXmYmXm$mXm!mXm#mXm*mXm&"
		"mXm%mYnlmYncmYm%mYmZmYm$mYm!mYm#mYm*mYm&m!m@m!m$m!nbm!m&m!m*m!m^m!nmm@"
		"m#m@nbm@nlm@m&m@m*m@m^m@nmm@n2m#m$m#nbm#nlm#nmm$nlm$m&m$m*m$m^m%m^m%m*"
		"m%nmm%n@m^m&m^nmm^nbm^n@m^nlm&m*m&nmm&nbm*"
		"nbn0nOn0nNn0nGn0nJn0nKn0nkn0npn0njn0non0nFn1uDn1uCn1uin1uhn1ucn1u6n1u5"
		"n1nYn1nXn1nQn1nPn1t@n1nTn1nUn1nwn1nAn1nxn1nCn1udn2obn2o9n2n%n2n#"
		"n2n4n2n@"
		"n2ocn2nmn2nvn2nnn2nun3pAn3pyn3pln3pjn3oUn3oLn3oEn3oAn3oon3omn3o7n3n6n3"
		"orn3o6n3onn3nrn3nyn3nqn3nzn3o5n4ogn4ofn4odn4o0n4n&n4nun4nnn4nmn4n#"
		"n4obn4n@n4ocn4n$n4oan4n^n5ojn5ohn5ogn5oen5nWn5nVn5nIn5nHn5n&"
		"n5o1n5ofn5okn5n^n5n*"
		"n5o2n5o4n6okn6ojn6oin6o3n6o1n6o5n6oon6o6n6onn6o8n6oln6o2n7nGn7nDn7nFn7"
		"n%n7njn7nnn7n@"
		"n7nKn8nPn8nQn8nSn8o9n8nun8nxn8ocn8nTn9nNn9nMn9nOn9o7n9non9nqn9o6n9nJna"
		"nYnanXnanZnaomnanznanCnaonnanUnbnlnbnmncnkncnlncnpndnpndnsndoKndoBneoA"
		"nenrnensneoKneoLneornenynfntnfnvngu5ngntngnwngudnguengt@"
		"nguCnhu6nhnAnhnBnhuinhulnhoSnhvDnht@"
		"nioEninyninBnioSnioUniulnivDniornjnGnjnknjnKnjnFnjnHnjnEnjnJnknKnknJnk"
		"nHnknpnmnnnmn@nnnGnnn@nnn%"
		"nnnDnonpnonJnonOnonInonLnonKnonNnpnJnpnKnpnInqnNnqnrnqo6nqo7nqnMnqoLnq"
		"oMnqpinroAnro6nroLnroMnrpinrornsoKnsoAnsoBnttWnttVnttUnttSntuentu5nunP"
		"nunvnuocnuo9nunSnvocnwu5nwnxnwnTnwudnwucnwuDnwt@"
		"nxnPnxnTnxnQnxudnxucnxuDnxt@"
		"nyoEnynznyonnyoUnyoTnypBnyornznYnzonnzomnznZnzoUnzoTnzpBnAu6nAnCnAnUnA"
		"uinAuhnAuCnAt@"
		"nBvunBvtnBu7nBoSnBulnBvDnBoEnBu6nBoFnCnYnCnUnCnXnCuinCuhnCuCnCt@nDn%"
		"nDn$nDnEnDnGnDo0nDn^nEn%nEnKnEo0nEn^nEn*nEnHnEn$nEn#nEnFnFn*"
		"nFnKnFnGnFnHnFo0nGn%nGnKnHo4nHo1nHn*nHn&nHo0nHnInHnKnIn*nIn&"
		"nIo4nIo1nIo3nInLnInJnInOnJo4nJnNnJnOnJnKnJnLnKn*"
		"nLo7nLo3nLo2nLo4nLo8nLo5nLnMnLnOnMo7nMo8nMnNnMo3nMo2nNnOnNo7nOo4nOo3nP"
		"nQnPnSnPo9nPnTnQoenQnTnQnRnQnWnQodnRo9nRodnRoenRognRnWnRoanRobnRnSnSo9"
		"nSoanSodnSognToenTnWnTudnTucnTt@nTuDnUohnUnYnUnVnUnXnUuinUuhnUt@"
		"nUuCnVofnVoenVohnVoinVok"
		"nVn!nVnWnVnXnWoknWohnWoenWodnWofnXohnXnYnXn!nXoinYnZnYomnZomnZolnZn!"
		"nZoinZojn!omn!oin!ohn!ojn!oln!oon@n#n@n%n#n&n#n$n#n^n#o0n$n%n$o0n$n^n$"
		"n*n%o0n^n&n^o0n&ofn&n*n&o1n*"
		"o0o1oko1o2o1o4o1o5o2o3o2o5o2o8o3o4o3o8o3o5o4o8o5o6o5o8o5pio5oro6pjo6o7"
		"o6oLo6oMo6pio7o8o7oro9oao9oco9odoaoboaodoaogoaoeobofobocobogobododoeod"
		"ogoeofofogoholohoiohokoiojoioloiooojokojooojolokooolomolooomonomoronpA"
		"onooonoUonoTonpBoopBoooropp5opp4opo^opp0opp1opoJopoOopoIopoNopo%oqv#"
		"oqv@oqvHoqvGoqvBoqvuoqpfoqpeoqp7oqp6oqvioqpaoqpboqoVoqoZoqoWoqo@"
		"oqvCorpAorpyorplorotorpiorpBoroLoroUoroMoroTorpjosqjosqiosqgosqbosq8os"
		"pXospNospLospwospuosoYosovosqcosqfosqdospvospMosoQosoXosoPotpFotpEotpC"
		"otppotpnotpjotpAotpiotpBotpkotpzotpmoupIoupGoupFoupDoupdoupcouo*ouo&"
		"oupnoupqoupEoupJoupmoupoouprouptovpJovpIovpHovpsovpqovoYovpuovpNovpvov"
		"pMovpxovpKovprowo^owo#owo%"
		"owplowoIowoMowpiowp1oxp6oxp7oxp9oxpyoxoToxoWoxpBoxpaoyp4oyp3oyp5oypwoy"
		"oNoyoPoypvoyp0ozpfozpeozpgozpLozoYozo@"
		"ozpMozpboAoKoAoLoAoUoBoJoBoKoBoOoCoOoCoRoCq7oCoJoDqeoDpWoDq8oDqboDq9oD"
		"oQoDoRoDq4oDq5oDq7oEoSoEoUoEvDoEuloFvuoFoSoFoVoFvCoFvDoFuloFvioGw*"
		"oGvvoGoZoGo!oGvHoGvKoGqkoGx0oGw^oHw*oHw^oHqgoHqjoHqhoHoXoHo!"
		"oHqnoHqmoHqkoIo^oIoJoIp1oIo%oIo&oIo$oIp0oJoKoJp1oJp0oJo&"
		"oJoOoLoMoLpioMo^oMpioMploMo#oNoOoNp0oNp5oNo*oNp2oNp1oNp4oOp0oOp1oOo*"
		"oPp4oPoQoPpvoPpwoPp3oPqfoPqcoPqeoQqboQq9oQpWoQpvoQqfoQqcoQqeoQq8oRpSoR"
		"q7oRq4oRqboRqaoRq8oSvtoSu7oSu6oSvDoSuloSvuoTp6oToUoTpBoTpyoTp9oUpBoVvu"
		"oVoWoVpaoVvCoVvBoVv#oVvioWp6oWpaoWp7oWvCoWvBoWv#"
		"oWvioXqmoXqfoXqdoXpYoXpXoXoYoXpMoXqjoXqioXqgoXqcoYqfoYpXoYpfoYpMoYpLoY"
		"pgoYqcoYqdoZvvoZo@oZpboZvHoZvGoZv@oZvio!x4o!x3o!x1o!vvo!qjo!qho!qgo!"
		"vKo!qko!qno!x0o!w^o!w*o@pfo@pbo@peo@vHo@vGo@v@o@vio#plo#pko#o$o#o^o#"
		"ppo#pmo$plo$p1o$ppo$pmo$poo$o&o$pko$pjo$o%o%poo%p1o%o^o%o&o%ppo^plo^"
		"p1o&pto&pqo&poo&pno&ppo&o*o&p1o*poo*pno*pto*pqo*pso*p2o*p0o*"
		"p5p0ptp0p4p0p5p0p1p0p2p1pop2pwp2psp2prp2ptp2pxp2pup2p3p2p5p3pwp3pxp3p4"
		"p3psp3prp4p5p4pwp5ptp5psp6p7p6p9p6pyp6pap7pDp7pap7p8p7pdp7pCp8pyp8pCp8"
		"pDp8pFp8pdp8pzp8pAp8p9p9pyp9pzp9pCp9pFpapDpapdpavCpavBpavipav#"
		"pbpGpbpfpbpcpbpepbvHpbvGpbvipbv@"
		"pcpEpcpDpcpGpcpHpcpJpcphpcpdpcpepdpJpdpGpdpDpdpCpdpEpepGpepfpephpepHpf"
		"pgpfpLpgpLpgpKpgphpgpHpgpIphpLphpHphpGphpIphpKphpNpipjpiplpjpppjpnpjpk"
		"pjpmpkplpkpppkpmpkpopmpnpmpppnpEpnpopnpqpopppqpJpqprpqptpqpuprpsprpupr"
		"pxpsptpspxpspuptpxpupvpupxpuqfpuqcpuqepvpwpvqfpvqcpvqepwpxpwqcpypzpypB"
		"pzpApzpCpzpFpzpDpApEpApCpApBpApFpCpDpCpFpDpEpEpFpGpKpGpHpGpJpHpIpHpKpH"
		"pNpIpJpIpNpIpKpJpNpKpLpKpNpLqdpLpMpLqcpMqfpMpXpMpNpMqcpMqdpNqdpNqcpOwM"
		"pOp@pOpQpOqmpOqlpOqnpOw*pOx0pOw&pOpPpOwLpOqhpOpYpOqgpOqipPw&"
		"pPpQpPwMpPp@pPwNpPp%pPwLpPqmpPqlpPpRpPqnpPpYpQpRpQwNpQp%pQwMpQp@"
		"pQxwpQwOpQxtpQxvpQp*pRwNpRqzpRqxpRqtpRq0pRp&pRp%"
		"pRqrpRxwpRqopRxvpRqqpRxtpRwOpRp*"
		"pRqspRqwpRqvpSpUpSq5pSq6pSq4pSqapSq9pSqbpSpTpSpWpSpZpTqDpTqbpTpUpTpZpT"
		"p#pTq5pTpVpTq6pTpWpTq4pUqIpUq6pUpVpUp#pUqApUqDpUpZpUqBpVq3pVq2pVp&"
		"pVqApVqBpVqDpVp^pVqLpVqHpVqEpVqKpVp#pVqIpVqGpWq6pWq4pWp!"
		"pWq9pWqepWq8pWqapWqfpWqdpWpXpWpZpWq5pXqhpXq8pXp@"
		"pXpZpXqdpXqepXqcpXpYpXp!pXqipXqjpYp!pYqipYqhpYqjpYp@"
		"pYqmpYqnpYqdpYqcpYqlpZq6pZq4pZp!pZp#pZp$pZqepZqdpZq9pZqap!p&p!p@p!p$p!"
		"qdp!qep!p#p!p%p!qcp!qip@p%p@p$p@qip@qhp@qjp@qmp#qDp#p$p#p^p#q3p#p&p#"
		"qIp#qLp#qAp$qJp$qyp$p%p$p&p$q0p$q3p$p^p$p*p$qzp$qIp%p*p%q0p%p&p%qwp%"
		"qzp%qrp^q0p^qIp^qLp^q3p^p&p^qEp^qAp^qBp^qHp^qFp^q2p&q0p&q3p&p*p&qwp&"
		"qLp*qYp*qop*q3p*qwp*qzp*q0p*qvp*qsp*qup*qrp*qqp*"
		"q1q0qzq0qyq0qwq0q1q0q3q0qrq1qWq1qYq1qyq1qxq1qzq1q2q1quq1qvq1qtq1qZq1qX"
		"q1qqq1qrq1qpq2qWq2qJq2qKq2qIq2q3q2qYq2qFq2qEq2qGq2qMq2qOq2qBq2qAq3qIq3"
		"qJq3qLq3qBq4q5q4q7q4qaq4qbq4q9q5q6q5qaq5qbq5q9q6q7q7qaq7qbq7q9q8q9q8qb"
		"q8qfq8qeq8qcq9qaq9qfq9qeq9qcqaqbqaqfqaqeqaqcqcqjqcqiqcqdqcqfqdqjqdqhqd"
		"qeqdqiqeqfqfqjqgqhqgqjqgqnqgqmqgqkqgx0qgw*"
		"qhqiqhqnqhqmqhqkqhx0qiqjqiqnqiqmqiqkqkx2qkvvqkqlqkqnqkx0qkw*"
		"qkvKqkwLqkx3qkx4qlqmqlw*qlx0qlw&"
		"qlwLqlvKqlvwqlwMqmqnqmwLqmwMqnvvqnx0qnvKqnx3qnx4qoxDqoxBqoqpqoqrqoxvqo"
		"xuqoxwqoxAqoxzqoxxqoqwqoqzqpxNqpxMqpxDqpxCqpqXqpqyqpqxqpqqqpqsqpxvqpqt"
		"qpxuqpqvqpxwqpxAqpqZqpxzqpxxqqxNqqxMqqxCqqqXqqqyqqqrqqqsqqqtqqqvqqqZqq"
		"xAqqxzqqqxqqqwqqxxqrqYqrqzqrqsqrxvqrqvqrqtqrxwqrxuqrqwqrqxqsxNqsxMqsxC"
		"qsxzqsqtqsqvqsqZqsqxqsqwqsqyqsxAqtxMqtqXqtquqtqZqtxzqtxAqtxNqtqYquxMqu"
		"qvquqZquqxquqwquqyquqYquxzquxNquqXqvqYqvqXqvqxqvqwqvqyqvxwqwqxqwqzqxqX"
		"qxqyqxqYqxqZqyqXqyqzqyqYqyqZqzqYqzxvqAqBqAqDqAqHqAqEqAqGqAqLqAqKqAqIqB"
		"qMqBqCqBqHqBqEqBqGqBqLqBqKqBqIqCqMqCqDqCqHqCqEqCqGqCqLqCqKqCqIqEqFqEqH"
		"qEqKqEqLqEqJqFqYqFqOqFqNqFqGqFqMqFqKqFqLqFqJqGqOqGqNqGqHqGqMqHqNqHqMqH"
		"qKqHqLqHqJqIqJqIqLqJqKqJqMqJqOqKqLqKqMqLqMqMqPqMqNqMqOqNqWqNqSqNqQqNqO"
		"qNqPqOqPqOqWqOqSqOqYqOqQqPqTqPqQqPqSqPqRqPqWqQqWqQqRqQqSqQq^qQq#"
		"qRqWqRq#qRq^qRqSqRqTqRq!qSr4qSq$qSqUqSqWqSqTqSq#qSqVqSq^"
		"qSr9qTrbqTqVqTqUqTr9qTqWqTr6qTxJqTq#qTxWqTqXqTq!"
		"qTxUqTxVqUxQqUxJqUr6qUxWqUqVqUr9qUxUqUxVqUxXqUxIqUr7qUqWqUxYqUqXqVqWqV"
		"xJqVqXqVr6qVxVqVxIqVxMqVr9qVxUqVxWqVxNqVqYqWrbqWqXqWqYqWxMqWqZqXxyqXqY"
		"qXqZqXxMqXxNqXxzqYqZqZxwqZxNqZxzqZxAqZxyqZxMqZxvqZxuq!raq!r4q!q&q!q$q!"
		"r8q!r9q!q*q!r1q!q@q!q#q!r5q!q%q@x@q@xZq@r8q@r5q@q*q@r2q@r7q@q&q@raq@x!"
		"q@q$q#q$q#q^q#q&q#r9q$r1q$q&q$q%q$r8q$r9q%rdq%rcq%r1q%q^q%r0q%q&q&raq&"
		"r4q&q*q&r8q&r1q&r5q&r9q*x!q*r5q*raq*r0q*r8q*r4q*rdq*"
		"r2r0r1r0rdr0rar0r5r1rdr2x#r2xYr2xWr2x!r2r3r2r7r2r5r2xZr2r8r2x@r2xXr3x!"
		"r3r4r3x@r3rbr3xZr3r7r3x#"
		"r3xXr3xYr4rbr4rar4r5r4rcr4Ggr5xXr5rdr5r7r5rar5rbr5r8r6x!"
		"r6xZr6xVr6xWr6r9r6xXr6r7r6xJr6xUr6xYr7x#r7x@r7xXr7r8r7xZr7x!"
		"r7xWr7xYr8x@r8x!"
		"r8xZr8rbr8r9r8rar9rbr9xJrarbrardrbrcrcrdrerfrernrerorerwrerpreryrerFre"
		"rqrerHrerrrerIrfrorfrnrfrprfrxrfrqrfryrfrGrfrHrfrzrfrIrgrhrgrqrgrprgrr"
		"rgryrgrzrgrxrgrArgrtrgrHrgrwrgrGrhrrrhrqrhrsrhrprhrtrhrArhryrhrBrhrorh"
		"rIrhrxrirjrirsrirrrirtrirkrirArirzrirBrirCrirlrirKrirDrjrkrjrsrjrlrjrr"
		"rjrArjrzrjrmrjrqrjrKrjrJrjy2rkrlrkrurkrtrkrmrkrCrkrvrkrBrkrArkrMrky2rl"
		"rmrlrvrly2rlrtrlrCrlrErlykrly3rlrMrmrvrmy2rmrErmy3rmrDrmycrmrOrmrNrmyd"
		"rnrornrwrnrxrnrprnrFrnrHrnrqrnrProrIrorzrorrrorprorxrorwroryrorqrorGro"
		"rFrorHrprXrprIrprGrprzrprqrpryrprxrprrrprHrprwrqrwrqrrrqryrqrzrqrxrqrH"
		"rqrGrrrArrryrrrBrrrIrrrxrsrtrsrArsrzrsrBrsrursrKrsrCrsrMrsrRrtrJrtrurt"
		"rBrtrArtrCrtrLrtrDrtrMruy2rurCrurBrurDrurMrurArurErurLrurOrurTrvrErvyb"
		"rvrDrvykrvrOrvycrvrCrvrNrvylrwrxrwrFrwrGrwryrwrPrxrJrxryrxrGrxrHrxrIry"
		"r!"
		"ryrXryrJryrFryrHryrGryrIryrPrzrArzrJrzrQrzrHrzrGrArKrArJrArRrArIrBrUrB"
		"rCrBrLrBrMrBrDrBrSrBrRrCr$rCrSrCrOrCrDrCrMrCrLrCrErCrTrDytrDybrDr%"
		"rDrKrDrErDrMrDrOrDykrDrLrDrVrEyKrEyurEy3rEr%"
		"rErUrErOrEykrErNrEybrErVrEylrFrGrFrHrFrPrFrIrFr&"
		"rGrJrGrHrGrPrGrIrGrZrHrQrHrIrHrPrHrJrHrYrIrJrIrPrIrYrJrQrJrPrJr@"
		"rJrTrKrLrKrRrKrSrKrMrKrTrKr#rKrNrLr!rLrMrLrSrLrRrLrNrLr@rLr#rMr%rMr$"
		"rMrRrMrNrMrTrMrUrMrOrMr#"
		"rNyLrNrOrNrTrNytrNykrNybrNyurOyTrOyKrOybrOrVrOytrOrUrOykrOr%"
		"rOyurPsdrPs6rPs5rPs4rPrYrPrZrPr*rPs0rPs1rQs7rQs5rQs4rQr^rQrRrQrZrQr@"
		"rQrSrQrYrQs0rQr*rQs2rQrTrRs2rRr$rRrSrRr!rRrTrRrZrRr#rRs1rSsarSs6rSr%"
		"rSrTrSr@rSr#rSrUrSr$rSs1rTsarTs9rTr!rTrUrTr#rTr$rTrVrTs3rTr@"
		"rUyTrUs2rUr!rUrVrUr#rUr%rUs3rUytrUyKrVyUrVr%rVr$"
		"rVytrVyKrVyurVyTrVs3rWr^rWr&rWr*rWs4rWs0rWsfrXslrXrYrXr&rXr*"
		"rXrZrXs0rXs4rXr!rXs1rXsfrXr@rYsdrYs6rYs5rYr@rYrZrYr*rYs0rYr!rYs4rYr^"
		"rYs1rZsmrZsfrZs7rZr^rZr!rZs0rZr*rZs1rZr@rZs5rZs4r!r@r!s2r!s7r!s3r@sgr@"
		"s1r@s2r@s3r@s6r#yTr#skr#sjr#r$r#s3r#s2r#s9r#sar$s3r$s2r$sbr$yTr$ytr%"
		"z1r%shr%yKr%yTr%sar%ytr%sjr^r&r^r*r^s0r^s1r&r*r&s4r&s0r&sfr&s1r&slr&"
		"sgr*slr*s7r*s0r*s4r*s1r*sdr*"
		"sfs0sns0scs0s1s0s5s0s4s0s6s0sfs1sos1sjs1s6s1s7s1sfs2s3s2s8s2sis2shs2sp"
		"s3yKs3sps3s9s3sas3sjs3sbs3sis4sts4sfs4sgs4sms5sts5s6s5sfs5sgs5s7s5sns5"
		"sos5s8s6sms6s7s6s8s6ses6sis6s9s7s8s7sgs7s9s7sjs7sks8sss8s9s8sis8shs8sj"
		"s8sps8sks8sns8sFs9sns9sj"
		"s9sis9shs9srs9sos9yTsasFsasjsasisasqsasssayTsaz0sasGsbsjsbyKsbyUsbz1sb"
		"z9sbsisbyVsbz2sbzhsbytscsuscshscsdscslscsmscsescstscsfscsgsdsesdsfsdsg"
		"sdsnsdshsesvsesusesfsesmsesgsesnsestseshsesisfslsfsisfsgsfshsfsmsgshsg"
		"sosgsisgsFsgsqshsishsnshspsispsisosisFsjsksjsssjz9sksrsksqskssskspsksG"
		"skz9skzasksFslsvslsmslstslsusmstsmsusmsvsnsvsnsusnsosnspsnsFsnsqsnstso"
		"sDsosvsostsospsosFsosqsosrsosssosEspsGspsEspsqspsFspsrspsssqsEsqsrsqss"
		"sqsFsqsGsqz9sqzhsrz1srsEsrsssrsGsrz9srz0srzhsrsFsrzasssFsssEsssGssz9ss"
		"z0sszassyTstsustsvstswsuszsusvsuswsusxsvsysvswsvsDsvszsvsFsvsxsvsEswsx"
		"swszswsyswsDswsJswsMswsAsxsysxszsxsMsxsJsxsDsysJsysMsyszsysAsysHsysYsy"
		"sDsysNszsDszsAszsJszsCszsMszsYszsKszsEsAsCsAsBsAsYsAsDsAAfsAsVsAA0sAAo"
		"sAzZsAAwsAsJsAApsAsEsBsDsBAfsBAosBAwsBsVsBAgsBApsBAxsBsCsBAFsBAGsBsYsB"
		"z!sBAysBAQsCA0sCzZsCsDsCA1sCz!sCsEsCzIsCAgsCAosCA2sCz@"
		"sCAwsCsVsDsEsDsFsDzZsDzIsDA0sDzzsEA0sEzzsEzIsEsFsEsGsEzqsEzhsEzAsEzJsE"
		"zZsFsGsGzZsGzhsGz9sGzisGz0sGzrsGzzsGzssGzIsHsNsHsKsHsXsHsYsHsOsHsQsHsI"
		"sHsJsHsUsHsLsIsKsIsXsIsUsIsOsIsRsIsWsIsNsIsZsIB6sIBfsJsKsJsMsJsNsJsYsK"
		"sQsKsNsKsLsKsXsKsYsLsQsLsMsLsPsLsNsNsZsNsOsNsXsNsQsNsUsNsYsOsUsOsZsOsP"
		"sOsXsOsTsOs#sOsRsPsQsPs#sPsZsPsUsQs#"
		"sRBfsRB6sRsSsRB7sRBvsRsWsRBDsRsUsRsXsSBvsSB6sSs$"
		"sSBDsSBusStCsStzsStvsStwsStBsStysSBEsTtnsTt5sTs!"
		"sTsZsTtysTtrsTtxsTtvsTsUsTtusTtssTt4sUB6sUsZsUs!"
		"sUsXsUtrsUtssVA0sVAwsVAFsVAosVsYsVAOsVAxsVAfsVAPsVsWsWBgsWAXsWB6sWsXsW"
		"AOsWAYsWB7sWBfsWAFsXBfsXsZsXsYsXB6sYAwsYAosZs!sZt5sZs#s!tqs!tns!trs!"
		"tus!tss!t4s!tys!txs!tvs@tus@s#s@tqs@tns@tps@tjs@tms@tks#tjs#tms#tks$"
		"BTs$tBs$tAs$tCs$s%s$tws$t5s$tvs$txs$t8s$B@s$s^s%B*s%tvs%s^s%t8s%tbs%"
		"tBs%tAs%s&s%C2s%BTs%tCs%t5s%BUs^s&s^tbs^t8s^BUs^B&s^B$s^C2s^C1s^B^s^B*"
		"s&tOs&tMs&tIs&tds&tbs&tGs&B&s&tDs&B^s&tFs&B$s&tes&BUs&tHs&tLs&tKs*tus*"
		"tts*t6s*t1s*tks*tls*tjs*tps*tos*tqs*t0s*"
		"t3t0tqt0t1t0t6t0t9t0tkt0t2t0tlt0t3t1t2t1t9t1tPt1tSt1t6t1tQt1tct2tdt2tP"
		"t2tQt2tSt2tct2t!"
		"t2tWt2tTt2tZt2t9t2tXt3t7t3tot3ttt3tnt3tpt3tut3tst3t4t3t6t3tkt3tjt3tlt4"
		"tnt4t8t4tst4ttt4trt4t5t4t7t4txt4tyt4twt4t6t5ttt5t7t5txt5twt5tyt5t8t5tB"
		"t5tCt5tst5trt5tAt6tut6tnt6tlt6t7t6t9t6tat6ttt6tst6tot6tpt7tdt7t8t7tat7"
		"tst7ttt7t9t7tbt7trt7txt8tAt8tbt8tat8txt8twt8tyt8tBt9tSt9tat9tct9tit9td"
		"t9tXt9t!"
		"t9tPtatbtatdtatftatitatctatetatOtatXtbtetbtftbtdtbtLtbtOtbtGtctXtct!"
		"tctitctdtctTtctPtctQtctWtctUtcthtdtftdtitdtetdtLtdt!"
		"tetDtetLtetOtetftetKtetHtetJtetGtetFtetgtftOtftNtftLtftgtftitfuotfuntf"
		"v5tgugtgu8tgtJtguotguntgv5tgv4tgt$"
		"tgtNtgtMtgtOthunthu4thtUthtithufthugthuUthuVtht$"
		"thtYthtZthtXtitXtitYtit!tiuftiugtiuU";

	const char coded_edges3[] =
		"tjtttjtktjtmtjtptjtqtjtotktltktptktqtktotltmtmtptmtqtmtotntotntqtntutn"
		"tttntrtotptotutotttotrtptrtptqtptutptttrtstrtutrtxtrtytstttstxtstytstw"
		"tttututytvBEtvBvtvtwtvtytvtCtvtBtvtztvBDtvButwButwtxtwtCtwtBtwtztwBDtx"
		"tytxtCtxtBtxtztyBDtztAtztCtzBDtzB#tzB@tzButzBZtzBTtAtBtABDtAB@tAB#tAB!"
		"tAButABTtBtCtBB@tBB!tBBTtCBwtCBDtCB#tCButCBZtDtOtDtEtDtGtDB^tDB%tDB&"
		"tDBOtDtLtDCKtDCHtDCOtECOtECNtECLtECJtEtNtEtMtEtFtEtHtEB^tEtItEB%"
		"tEtKtEB&"
		"tEumtECKtECHtEBOtFCJtFtNtFtGtFtHtFtItFtKtFumtFtMtFtLtFu8tFCKtFCHtGtOtG"
		"tHtGB^tGtKtGtItGB&tGB%"
		"tGtLtGtMtGBUtHCJtHCHtHtItHtKtHumtHtMtHtLtHtNtHCKtItJtIu8tIumtICKtICJtI"
		"CHtIuotJtKtJu8tJtMtJumtJtLtJtNtJuotJCKtJCJtKu8tKtMtKtLtKtNtKuotLtMtLtO"
		"tLu8tMtNtMuotMu8tNtOtNuotNu8tNv5tOuotOv5tPtQtPtStPtWtPtTtPtVtPt!"
		"tPtZtPtXtQtXtQtRtQtWtQtTtQtVtQt!tQtZtRtXtRtStRtWtRtTtRtVtRt!"
		"tRtZtRuetTtUtTtWtTu4tTtZtTt!tTtYtTuftUtVtUu4tUuetUtZtUt!tUtYtUuftUt$"
		"tVtWtVu4tVuetVuftWuetWtZtWt!tWtYtXtYtXt!"
		"tXuftXuUtYtZtYuftYu4tYuUtYuetZt!tZuftZu4t!u4t@uHt@uGt@uzt@uyt@uCt@uDt@"
		"udt@uit@uct@uht#D8t#D7t#D5t#Cqt#uRt#uQt#uJt#uIt#uMt#uNt#upt#utt#uqt#"
		"uvt#Cpt#Cot$v4t$v2t$uXt$uVt$t^t$uUt$v5t$uft$uot$ugt$unt%wtt%wrt%wet%"
		"wct%vNt%vEt%vxt%vtt%vht%vft%v0t%t*t%vkt%u*t%vgt%ukt%urt%ujt%ust%u&t^"
		"v9t^v8t^v6t^u@t^uZt^unt^ugt^uft^uVt^v4t^uUt^v5t^uWt^v3t^uYt&vct&vat&"
		"v9t&v7t&uPt&uOt&uBt&uAt&uZt&u#t&v8t&vdt&uYt&u!t&u$t&u^t*vdt*vct*vbt*u%"
		"t*u#t*u&t*vht*u*t*vgt*v1t*vet*u$"
		"u0uzu0uwu0uyu0uXu0ucu0ugu0uUu0uDu1uIu1uJu1uLu1v2u1unu1uqu1v5u1uMu2uGu2"
		"uFu2uHu2v0u2uhu2uju2u*"
		"u2uCu3uRu3uQu3uSu3vfu3usu3uvu3vgu3uNu4ueu4ufu5udu5ueu6uiu6ulu6vDu7vtu7"
		"uku7ulu7vDu7vEu7vku8umu8uou9CJu9Cnu9CIu9CKu9umu9upu9Cou9CNuaDHuaDduaD7"
		"uaD8uaD6uautuauuuaDcuaDbuaD9ubvxuburubuuubvLubvNubDIubDFubvkucuzucuduc"
		"uDucuyuduDufugufuUuguzuguUuguXuguwuhuGuhuiuhuCuhuHuiuCujuGujukuju*"
		"ujv0ujuFujvEujvFujwbukvtuku*"
		"ukvEukvFukwbukvkulvuulvDulvtumCOumCNumCMumCnumCKumCJumCHunuIunuounv5un"
		"v2unuLuov5upCJupCIupuqupuMupCoupCpupCnupuNuquIuquMuquJuqCouqCpuquPuquK"
		"urvxurusurvgurvNurvMurwuurvkusuRusvgusvfusuSusvNusvMuswuutCputCoutuvut"
		"uNutD8utD5utD7utCquuDLuuDJuuD8uuD7uuD6uuvyuuvxuuvLuuDIuuDFuuDHuuDbuuDc"
		"uvCouvuRuvuNuvuQuvCpuvuOuvuTuvuMuwuXuwuWuwuxuwuzuwu@uwuYuxuXuxu@"
		"uxuYuxu!uxuAuxuWuxuVuxuyuyu!uyuDuyuzuyuAuyu@uzuXuzuDuAu^uAu#uAu!"
		"uAuZuAu@uAuBuAuDuBu!uBuZuBu^uBu#uBu%uBuEuBuCuBuHuCu^uCuGuCuHuDu!"
		"uEv0uEu%uEu$uEu^uEv1uEu&uEuFuEuHuFv0uFv1uFuGuFu%uFu$uGuHuGv0uHu^uHu%"
		"uIuJuIuLuIv2uIuMuJv7uJuMuJuKuJuPuJv6uKv2uKv6uKv7uKv9uKuPuKv3uKv4uKuLuL"
		"v2uLv3uLv6uLv9uMv7uMCouMuNuMuPuMCpuNCouNvauNuRuNuQuNuOuNCquNCpuOv8uOv7"
		"uOvauOvbuOvduOuTuOuPuOuQuPvduPvauPv7uPv6uPv8uQvauQuRuQuTuQvbuRuSuRvfuS"
		"vfuSveuSuTuSvbuSvcuTvfuTvbuTvauTvcuTveuTvhuUuVuUuXuVuZuVuWuVuYuVu@"
		"uWuXuWu@uWuYuWu!uXu@uYuZuYu@uZv8uZu!uZu#u!u@u#vdu#u$u#u^u#u&u$u%u$u&u$"
		"v1u%u^u%v1u%u&u^v1u&u*u&v1u&wbu&vku*wcu*v0u*vEu*vFu*"
		"wbv0v1v0vkv2v3v2v5v2v6v3v4v3v6v3v9v3v7v4v8v4v5v4v9v4v6v6v7v6v9v7v8v8v9"
		"vavevavbvavdvbvcvbvevbvhvcvdvcvhvcvevdvhvevfvevhvfvgvfvkvgwtvgvhvgvNvg"
		"vMvgwuvhwuvhvkviv&viv^vivYvivXviv@viv#"
		"vivCvivHvivBvivGvjE6vjE5vjE3vjDovjw8vjw7vjw0vjv*"
		"vjw3vjw4vjvOvjvSvjvPvjvUvjDnvjDmvkwtvkwrvkwevkvmvkwbvkwuvkvEvkvNvkvFvk"
		"vMvkwcvlxcvlxbvlx9vlx1vlwQvlwGvlwEvlwpvlwnvlvRvlvovlx5vlx8vlx6vlwovlwF"
		"vlvJvlvQvlvIvmwyvmwxvmwvvmwivmwgvmwcvmwtvmwbvmwuvmwdvmwsvmwfvnwBvnwzvn"
		"wyvnwwvnw6vnw5vnv!"
		"vnvZvnwgvnwjvnwxvnwCvnwfvnwhvnwkvnwmvowCvowBvowAvowlvowjvovRvownvowGvo"
		"wovowFvowqvowDvowkvpvYvpvVvpvXvpwevpvBvpvFvpwbvpv#vqv*"
		"vqw0vqw2vqwrvqvMvqvPvqwuvqw3vrv^vrv%vrv&vrwpvrvGvrvIvrwovrv@"
		"vsw8vsw7vsw9vswEvsvRvsvUvswFvsw4vtvDvtvEvuvCvuvDvvvHvvvKvvx0vwx7vwwPvw"
		"x1vwx4vwx2vwvJvwvKvww^vww&"
		"vwx0vxvLvxvNvxDbvxDcvyDHvyDlvyDGvyDIvyvLvyvOvyDmvzxdvzE5vzE6vzE4vzvSvz"
		"vTvzEavzE9vzE7vAwHvAx9vAxcvAxavAvQvAvTvAxgvAxfvAxdvBvYvBvCvBv#vBvXvCv#"
		"vEvFvEwbvFvYvFwbvFwevFvVvGv^vGvHvGv@vGv&vHv@vIv^vIvJvIwovIwpvIv%"
		"vIx8vIx5vIx7vJx4vJx2vJwPvJwovJx8vJx5vJx7vJx1vKx1vKx0vKw^"
		"vKx4vKx3vLDMvLDLvLDKvLDlvLDcvLDavLD8vLDIvLDHvLDFvLDbvMv*"
		"vMvNvMwuvMwrvMw2vNwuvODHvODGvOvPvOw3vODmvODnvODlvOw4vPv*"
		"vPw3vPw0vPDmvPDnvPw6vPw1vQxfvQx8vQx6vQwRvQwQvQvRvQwFvQxcvQxbvQx9vQx5vR"
		"x8vRwQvRw8vRwFvRwEvRw9vRx5vRx6vSDnvSDmvSvUvSw4vSE6vSE3vSE5vSDovTE6vTE4"
		"vTxcvTxavTwHvTxdvTxgvTE9vTEavTE5vTx9vUDmvUw8vUw4vUw7vUDnvUw5vUwavUw3vV"
		"wevVwdvVvWvVvYvVwivVwfvWwevWwivWwfvWwhvWvZvWwdvWwcvWvXvXwhvXv#"
		"vXvYvXvZvXwivYwevYv#vZwmvZwjvZwhvZwgvZwivZv!vZv#v!whv!wgv!wmv!wjv!wlv!"
		"v$v!v@v!v&v@wmv@v^v@v&v#whv$wpv$wlv$wkv$wmv$wqv$wnv$v%v$v&v%wpv%wqv%v^"
		"v%wlv%wkv^v&v^wpv&wmv&wlv*w0v*w2v*wrv*"
		"w3w0www0w3w0w1w0w6w0wvw1wrw1wvw1www1wyw1w6w1wsw1wtw1w2w2wrw2wsw2wvw2wy"
		"w3E6w3www3Dmw3w4w3w6w3Dnw4Dmw4wzw4w8w4w7w4w5w4Dow4Dnw5wxw5www5wzw5wAw5"
		"wCw5waw5w6w5w7w6wCw6wzw6www6wvw6wxw7wzw7w8w7waw7wAw8w9w8wEw9wEw9wDw9wa"
		"w9wAw9wBwawEwawAwawzwawBwawDwawGwbwcwbwewcwiwcwgwcwdwcwfwdwewdwiwdwfwd"
		"whwfwgwfwiwgwxwgwhwgwjwhwiwjwCwjwkwjwmwjwnwkwlwkwnwkwqwlwmwlwqwlwnwmwq"
		"wnwownwqwnx8wnx5wnx7wowpwox8wox5wox7wpwqwpx5wrwswrwuwswtwswvwswywswwwt"
		"wxwtwvwtwuwtwywvwwwvwywwwxwxwywzwDwzwAwzwCwAwBwAwDwAwGwBwCwBwGwBwDwCwG"
		"wDwEwDwGwEx6wEwFwEx5wFx8wFwQwFwGwFx5wFx6wGx6wGx5wHwJwHxfwHxewHxgwHwIwH"
		"xa"
		"wHwRwHx9wHxbwHwUwHEjwIEpwIwJwIEjwIwUwIwXwIxfwIxewIEswIwKwIErwIEAwIxgwJ"
		"wKwJwXwJEjwJEAwJExwJwUwJEzwJErwJEswJw!wKxswKxmwKw@"
		"wKwZwKEzwKEAwKxkwKxhwKxjwKw!wKxlwKxpwKxowKxqwKwXwLwSwLwNwLw&wLw*wLw^"
		"wLx3wLx2wLx4wLwMwLwPwMwNwMwSwMwVwMw&wMwOwMw*"
		"wMwPwNwOwNwVwNxtwNxwwNwSwNxuwNwYwOw%"
		"wOwZwOxtwOxuwOxwwOwYwOxEwOxAwOxxwOxDwOwVwOxBwPw*wPw^"
		"wPwTwPx2wPx7wPx1wPx3wPx8wPx6wPwQwPwSwPw&"
		"wQxawQx1wQwUwQwSwQx6wQx7wQx5wQwRwQwTwQxbwQxcwRwTwRxbwRxawRxcwRwUwRxfwR"
		"xgwRx6wRx5wRxewSwTwSwVwSwWwSx7wSx6wSx2wSx3wTwZwTwUwTwWwTx6wTx7wTwVwTwX"
		"wTx5wTxbwUwXwUwWwUxbwUxawUxcwUxfwVxywVxwwVwWwVwYwVw%"
		"wVwZwVxBwVxEwVxtwWxCwWxrwWwXwWwZwWw@wWw%wWwYwWw!wWxswWxBwXw!wXw@"
		"wXwZwXxpwXxswXEAwYw@wYxBwYxEwYw%wYwZwYxxwYxtwYxuwYxAwYxywYw$wZw@wZw%"
		"wZw!wZxpwZxEw!xhw!w%w!xpw!xsw!w@w!xow!xlw!xnw!xkw!xjw!w#w@xsw@xrw@xpw@"
		"w#w@w%w@xkw#xRw#xGw#xrw#xKw#xqw#xsw#w$w#xnw#xow#xmw#xLw$xCw$xDw$xKw$"
		"xBw$w%w$xyw$xxw$xzw%xBw%xCw%xEw%xuw^w&w^x0w^x3w^x4w^x2w&w*w&x3w&x4w&"
		"x2w*"
		"x0x0x3x0x4x0x2x1x2x1x4x1x8x1x7x1x5x2x3x2x8x2x7x3x7x3x4x3x8x5xcx5xbx5x6"
		"x5x8x6xcx6xax6x7x6xbx7x8x8xcx9xax9xcx9xgx9xfx9xdx9E9x9Eaxaxbxaxgxaxfxa"
		"xdxbxcxbxgxbxfxbxdxdE6xdxexdxgxdE9xdEaxdE8xdEfxexfxeE9xeEaxeE8xeEfxeEj"
		"xeEHxeEIxfxgxfEfxfEjxgE9xgEaxgE8xhEDxhxixhxkxhElxhEzxhEyxhEAxhE!"
		"xhxpxhxsxhEExiEDxixFxixrxixjxixlxixmxiElxixoxiEzxiEyxiEAxiE!"
		"xixqxiEExjxkxjxlxjxmxjxoxjxqxjxpxjE!"
		"xjxrxjxFxkxFxkxsxkxlxkxoxkxmxkEzxkEyxkEAxkxpxkxqxlxmxlxoxlxqxlxpxlxrxl"
		"xFxlElxmEZxmxGxmxnxmxFxmE!xmElxmEDxmEExnxoxnxqxnxpxnxrxnxFxnE!"
		"xnxGxnxKxnEZxoxKxoxGxoxqxoxpxoxrxoElxpxqxpxsxqxLxqxrxqxFxqxKxrxsxrxFxr"
		"xLxsEzxsEAxtxuxtxwxtxAxtxxxtxzxtxExtxDxtxBxuxBxuxvxuxAxuxxxuxzxuxExuxD"
		"xvxwxvxAxvxxxvxzxvxExvxDxvxBxxxyxxxAxxxDxxxExxxCxyxzxyxDxyxExyxCxyxNxy"
		"xKxzxAxzxNxAxNxAxDxAxExAxCxBxCxBxExCxDxCxNxDxKxDxExFE%"
		"xFxRxFxLxFxKxFEZxFE!xFxGxFxOxGE%"
		"xGxIxGxLxGxKxGxOxGxRxGEZxHxQxHxTxHF7xHF8xHy1xHxSxHF9xHxPxHFaxHE$"
		"xHy0xIxWxIxQxIxVxIxRxIxLxIxMxIxJxIxSxJxYxJxRxJxQxJxUxJxVxJxWxJxXxKxLxL"
		"xRxLxMxLxOxMxRxMxQxMxNxOxPxOEZxOE%xOE$xOF8xOxRxOEYxPxSxPE$xPF8xPE%"
		"xPEZxPxTxPF7xPxQxQxRxQxVxQxSxQF8xRxVxSxTxSy1xSF8xSxVxSF7xSF9xTy0xTy1xT"
		"F7xTF8xTF9xTxUxTx*xUxVxUxWxUy0xUxYxUxXxVxXxVxWxWxXxWxYxWxZxWy0xWx!xXx@"
		"xXxYxXxZxXx!xXx#xXx&xXy0xYxZxYx&xYx#xYy0xYx!xYx*xYx@xZx!xZx#xZx@xZx&"
		"xZx$xZy0x!x@x!x#x!x&x!x$x@x#x@x$x@x&x@x^x@x%x@G7x#x&x#x$x#x^x#x*x#x%x#"
		"y0x#F!x$x^x$x%x$x&x$G7x$F!x$Ggx$FRx$Ghx$x*x$GYx$FSx$Grx%G7x%Ggx%G@x%"
		"G8x%Ghx%GYx%x^x%Grx%FSx%Gox%x&x%FAx%Gjx^F!x^FRx^x&x^F@x^FSx^x*x^FAx^"
		"G8x^Ggx^F#x^FTx^G@x&x*x&y0x&FRx&FAx&F!x&Frx&y1x*Frx*FAx*y0x*y1x*Fix*"
		"F9x*Fsx*FBx*FRx*Fax*FSx*F!y0y1y0F9y0Fiy0Fry0FAy0F7y0Fay0Fjy1FRy1E$"
		"y1F9y1Fay1F7y1Fjy1Fry1F8y1Fky1FAy1Fty1Fly2y3y2yby2ycy2yky2ydy2ymy2yty2"
		"yey3ycy3yby3ydy3yly3yey3ymy3yuy3yvy4y5y4yey4ydy4yfy4ymy4yny4yly4yoy4yh"
		"y4yvy4yky4yuy5yfy5yey5ygy5ydy5yhy5yoy5ymy5ypy5ycy5ywy5yly6y7y6ygy6yfy6"
		"yhy6y8y6yoy6yny6ypy6yqy6y9y6yyy6yry7y8y7ygy7y9y7yfy7yoy7yny7yay7yey7yy"
		"y7yxy7Hxy8Huy8Hmy8y9y8yiy8yhy8yay8yqy8yjy8ypy8yoy8yAy8Hxy9Hmy9yay9yjy9"
		"Hxy9yhy9yqy9ysy9Huy9yAy9yCyaHmyayjyaHxyaHwyaHuyaysyayryayCyayByaG%"
		"ybyeybycybykybylybydybytycywycyvycynycyfycydycylycykycymycyeycyuycytyd"
		"ywydyuydynydyeydymydylydyfydyvydykyeyfyeymyeynyeylyeyvyeyuyfyoyfymyfyp"
		"yfywyfylygyhygyoygynygypygyiygyyygyqygyAygyFyhyxyhyiyhypyhyoyhyqyhyzyh"
		"yryhyAyiHxyiyqyiypyiyryiyAyiyoyiysyiyzyiyCyiyHyjyJyjysyjHwyjyryjHvyjyC"
		"yjyqyjyByjyAyjG%"
		"ykyDykylykytykyuykymylyxylymylyuylyvylywymyOymyLymyxymytymyvymyuymywym"
		"yDynyoynyxynyEynyvynyuyoyByoyyyoyxyoyFyoywypyIypyqypyzypyAypyrypyGypyF"
		"yqyRyqyGyqyCyqyryqyAyqyzyqysyqyHyrySyryyyrysyryAyryCyryzyryJyrHwysH2ys"
		"G%"
		"ysySysyIysyCysyBysHwysH5ysyJysHvysHxytyUytyDytywytyuytyvyuyxyuyvyuyDyu"
		"ywyvyEyvywyvyDyvyxyvyMywyxywyDywyMyxyEyxyDyxyPyxyHyyyzyyyFyyyGyyyAyyyH"
		"yyyQyyyByzyOyzyAyzyGyzyFyzyByzyPyzyQyAySyAyRyAyFyAyByAyHyAyIyAyCyAyQyB"
		"yCyByHyBH5yBHwyBG%yCHxyCHvyCH4yCH2yCyJyCH5yCyIyCySyCG%yCHwyDy#yDy@yDy!"
		"yDyMyDyNyDyVyDyWyDyXyEy$yEy@"
		"yEyFyEyNyEyPyEyGyEyMyEyWyEyVyEyYyEyHyFyYyFyRyFyGyFyOyFyHyFyNyFyQyFyXyG"
		"y&yGy#yGySyGyHyGyPyGyQyGyIyGyRyGyXyHy&yHy^"
		"yHyOyHyIyHyQyHyRyHyJyHyZyHyPyIyYyIyOyIyJyIyQyIySyIH5yIH4yIyZyJH5yJySyJ"
		"yRyJH4yJG%yJyZyKyTyKyUyKyVyKy!yKyWyLyMyLyUyLyVyLyNyLyWyLy!"
		"yLyOyLyXyLz3yLyPyMz1yMy#yMy@yMyPyMyNyMyVyMyWyMyOyMy!yMyTyMyXyNz3yNy$"
		"yNyOyNyWyNyVyNyXyNyPyNy@yNy!yOyPyOyYyOy$yOyZyPz4yPy&yPyXyPyYyPyZyPy#"
		"yQz8yQz7yQyRyQyZyQyYyQy^yQy&yRyZyRH5yRyYyRy*yRHlySH4ySH5ySy&"
		"ySHlySz7yTyUyTyVyTyWyUyVyUy!yUyWyUz3yUyXyUz9yVy$yVyWyVy!"
		"yVyXyVz1yVz3yWyXyWy@yWy!yWy#yWz3yXz7yXy#yXy$yXz3yYyZyYy%"
		"yYz6yYz5yYzdyZy^yZy&yZz7yZy*yZz6y!z3y!z4y!zay!ziy@y#y@z3y@z4y@y$y@zby@"
		"zcy@y%y#y$y#y%y#z2y#z6y#y^y$y%y$z4y$y^y$z7y$z8y%y^y%z6y%z5y%z7y%zdy%"
		"z8y%zby%zmy^z7y^z6y^z5y^zfy^zcy^zby&z7y&z6y&zey&zgy&Hky&zpy*H4y*Hly*"
		"Hky*z7y*H3y*z6y*H5y*HZy*H$y*"
		"HWz0z1z0z9z0zaz0z2z0zhz0z3z0zqz0zrz1z2z1z3z1z4z1zbz1zkz1z5z2z3z2zaz2z4"
		"z2zbz2ziz2zjz2z5z2zrz3z4z3z5z3zaz4z5z4zcz4z6z4zjz4zez5z6z5zbz5zdz5zkz5"
		"zmz5zjz5znz6zkz6zdz6zcz6zmz6znz7z8z7zgz7zoz8zfz8zez8zgz8zoz8zdz8zpz8zm"
		"z8HZz8HYz9zzz9zaz9zhz9ziz9zqz9zszazAzazizazrzazjzazqzazszbzezbzczbzkzb"
		"zjzbzlzbzdzbzuzbztzbzszczdzczlzczkzczezczvzczjzcztzdzgzdzezdzmzdznzdzf"
		"zdzxzezfzezmzezozezgzezpzezvzezyzfzgzfzozfznzfzpzfHZzfzyzfHYzfHWzgzpzg"
		"zozgzyzgHkzgznzgHlzhzizhzqzhzrzhzzzhzszhzBzhzIzizrzizqzizszizAziztzizB"
		"zizJzjzAzjzkzjztzjzszjzuzjzBzjzCzkzuzkztzkzvzkzszkzwzkzDzkzBzlzNzlzmzl"
		"zvzlzuzlzwzlznzlzDzlzCzlzEzlzFzlzozmzNzmznzmzvzmzozmzuzmzDzmzCznHZznHY"
		"znzEznzoznzxznzwznzpznzFznzyzoH$zozFzozwzozpzozyzoHZzoHYzoHWzpH@"
		"zpzQzpzGzpHZzpzyzpHYzpHWzpzHzpHCzpH#zpH$"
		"zqzrzqzzzqzAzqzszqzIzqzKzrzLzrzKzrzCzrzuzrzszrzAzrzzzrzBzrztzrzJzrzIzs"
		"zLzszJzszCzszzzsztzszBzszAzszuzszKztzJztzuztzBztzCztzAztzKzuzDzuzBzuzE"
		"zuzLzvzwzvzDzvzCzvzEzvzxzvzNzvzFzwzMzwzxzwzEzwzDzwzFzwzOzwzGzwzPzxzFzx"
		"zEzxzGzxzPzxzDzxzHzxzOzxzRzyHDzyzHzyzGzyHCzyzRzyzFzyzQzzzAzzzIzzzJzzzB"
		"zzzSzAzMzAzBzAzJzAzKzAzLzBz$zBz!"
		"zBzMzBzIzBzKzBzJzBzLzBzSzCzDzCzMzCzTzCzKzCzJzDzNzDzMzDzUzDzLzEzXzEzFzE"
		"zOzEzPzEzGzEzVzEzUzFzVzFzRzFzGzFzPzFzOzFzHzFzWzGHXzGHDzGzNzGzHzGzPzGzR"
		"zGzOzGzYzGHCzHH$zHH#zHz*"
		"zHzXzHzRzHzQzHHCzHHYzHHXzHHZzHzYzHHDzIzJzIzKzIzSzIzLzIA1zJzMzJzKzJzSzJ"
		"zLzJz#zKzTzKzLzKzSzKzMzKz@zLzMzLzSzLz@zMzTzMzSzMz%"
		"zMzWzNzOzNzUzNzVzNzPzNzWzNz^zNzQzOz$zOzPzOzVzOzUzOzQzOz%zOz^zPz*zPz&"
		"zPzUzPzQzPzWzPzXzPzRzPz^"
		"zQzRzQzWzQHDzQHCzQHEzQHYzRHZzRHYzRHXzRzYzRzXzRHDzRz*"
		"zRHCzRHEzSAgzSA9zSA8zSA7zSz@zSz#zSA2zSA3zSA4zTAazTA8zTA0zTzUzTz#zTz%"
		"zTzVzTz@zTA3zTA2zTA5zTzWzUA5zUzVzUz$zUzWzUz#zUz^zUA4zVA9zVzWzVz%zVz^"
		"zVzXzVz&zVA4zWAdzWAczWz$zWzXzWz^zWz&zWzYzWA6zWz%zXA5zXz$zXzYzXz^zXz*"
		"zXA6zXHDzXHEzYInzYz*zYz&"
		"zYHDzYHEzYA6zYHCzYHFzZA0zZA1zZA2zZA7zZA3zZAizZAaz!z@z!A1z!A2z!z#z!A3z!"
		"A7z!z$z!A4z!Aiz!z%z@Agz@A9z@A8z@z%z@z#z@A2z@A3z@z$z@A7z@A0z@A4z#Aiz#"
		"Aaz#A0z#z$z#A3z#A2z#A4z#z%z#A8z#A7z$z%z$A5z$Aaz$A6z%Ajz%A4z%A5z%A6z%"
		"A9z^Inz^Anz^Amz^z&z^A6z^A5z^Acz^Adz&A6z&A5z&Aez&HEz&Ikz&HDz&HFz*HEz*"
		"Inz*HDz*HFz*Ikz*Adz*"
		"ImA0A1A0A2A0A3A0A4A1A2A1A7A1A3A1AiA1A4A1AoA1AjA2AaA2A3A2A7A2A4A2AgA2Ai"
		"A3AfA3A4A3A8A3A7A3A9A3AiA4A9A4AaA4AiA5A6A5AbA5AlA5AkA5AsA6InA6AcA6AdA6"
		"AmA6AeA6AlA7AiA7AjA7ApA7AxA8A9A8AiA8AjA8AaA8AqA8ArA8AbA9AaA9AbA9AhA9Al"
		"A9AcAaAbAaAjAaAcAaAmAaAnAbAcAbAlAbAkAbAmAbAsAbAnAbAqAbABAcAmAcAlAcAkAc"
		"AuAcArAcAqAdAmAdAlAdAtAdAvAdAEAdIrAdIqAdIoAeImAeInAeHFAeAmAeIrAeIoAeIq"
		"AeHEAeIRAeAlAfAgAfAoAfApAfAhAfAwAfAiAfAFAfAGAgAhAgAiAgAjAgAqAgAzAgAkAh"
		"AiAhApAhAjAhAqAhAxAhAyAhAkAhAGAhAIAiAoAiAjAiAkAiApAjAkAjArAjAlAjAyAjAt"
		"AkAlAkAqAkAsAkAzAkABAkAyAkACAlAzAlAsAlArAlABAlACAmAnAmAvAmADAmInAmIRAn"
		"AuAnAtAnAvAnADAnAsAnAEAnABAnIrAnIqAoApAoAwAoAxAoAFAoAHAoAOApAxApAGApAy"
		"ApAFApAHApAPAqAtAqArAqAzAqAyAqAAAqAsAqAJAqAIAqAHArAsArAAArAzArAtArAKAr"
		"AyArAIAsAtAsABAsACAsAuAsAMAtAuAtABAtADAtAvAtAEAtAK"
		"AtANAuAvAuADAuACAuAEAuIRAuANAuIrAuALAvInAvACAvAEAvADAvIqAvIrAvANAvImAv"
		"IlAwAxAwAFAwAGAwAOAwAHAwAQAwAXAwAIAxAGAxAFAxAHAxAPAxAIAxAQAyAPAyAzAyAI"
		"AyAHAyAJAyAQAyARAzAJAzAIAzAKAzAHAzALAzASAzAQAAA#"
		"AAABAAAKAAAJAAALAAACAAASAAARAAATAAAUAAADABA#"
		"ABACABAKABADABAJABASABARACATACADACAMACALACAEACAUACANADIIADIpADAWADAEAD"
		"ANADIRADALADAUAEA^"
		"AEAVAEIRAEANAEIqAEAWAEIrAEIpAFAGAFAOAFAPAFAHAFAXAFAZAGA!"
		"AGARAGAJAGAHAGAPAGAOAGAQAGAIAGAYAGAXAGAZAHA!"
		"AHAYAHARAHAOAHAIAHAQAHAPAHAJAHAZAIAYAIAXAIAJAIAQAIARAIAPAIAZAJASAJAQAJ"
		"ATAJA!AKALAKASAKARAKATAKAMAKA#AKAUALA@ALAMALATALASALAUALA$ALAVALA%"
		"AMAUAMATAMAVAMA%AMASAMAWAMA$AMA&ANAWANIIANAVANIRANA&ANAUANA^"
		"AOAPAOAXAOAYAOAQAOA*APA@APAQAPAYAPAZAPA!AQBaAQB7AQA@AQAXAQAZAQAYAQA!"
		"AQA*ARASARA@ARB0ARAZARAYASA#ASA@ASB1ASA!ATAUATA$ATA%"
		"ATAVATB2ATB1AUB2AUAVAUA%AUA$AUAWAUB3AVA#AVAWAVA%AVA&AVIIAVIQAVA$AWA&"
		"AWIIAWA^AWIQAWB5AWB4AXAYAXAZAXA*AXA!AXBgAYA@AYAZAYA*AYA!AYB9AZB0AZA!"
		"AZA*AZA@AZB8A!A@A!A*A!B8A@B0A@A*A@BbA@B3A#A$A#B1A#B2A#A%A#B3A#BcA#A^A$"
		"BaA$A%A$B2A$B1A$A^A$BbA$BcA%BdA%B1A%A^A%B3A%B4A%A&A%BcA^A&A^B3A^IQA^"
		"IIA^J8A^IwA&IQA&B5A&B4A&J8A&IIA&IwA&BeA*BvA*BoA*BnA*BmA*B8A*B9A*BhA*"
		"BiA*"
		"BjB0BpB0BnB0BmB0BfB0B1B0B9B0BbB0B2B0B8B0BiB0BhB0BkB0B3B1BkB1B2B1BaB1B3"
		"B1B9B1BcB1BjB2BsB2BoB2B3B2BbB2BcB2B4B2BdB2BjB3BsB3BrB3BaB3B4B3BcB3BdB3"
		"B5B3BlB3BbB4BkB4BaB4B5B4BcB4BeB4IwB4IQB4BlB4IPB5IUB5BlB5BeB5IwB5IQB5Bd"
		"B5IPB5J8B5J7B5IVB6BfB6BgB6BhB6BmB6BiB6BxB7B8B7BgB7BhB7B9B7BiB7BmB7BaB7"
		"BjB7BxB8BvB8BoB8BnB8BbB8B9B8BhB8BiB8BaB8BmB8BfB8BjB9BpB9BfB9BaB9BiB9Bh"
		"B9BjB9BbB9BnB9BmBaBbBaBkBaBpBaBlBbByBbBjBbBkBbBlBbBoBcBCBcBBBcBdBcBlBc"
		"BkBcBrBcBsBdBlBdIwBdIVBdBkBdBtBdIQBdJ8BeBBBeIwBeIVBeIUBeJ7BeJ8BeIQBeBs"
		"BeIPBeIJBfBgBfBhBfBiBfBjBgBhBgBmBgBiBgBxBgBjBgBDBgByBhBpBhBiBhBmBhBjBh"
		"BvBhBxBiBjBiBnBiBmBiBoBiBxBjBoBjBpBjBxBkBlBkBqBkBABkBzBkBHBlIJBlBrBlBs"
		"BlBBBlBtBlBABmBxBmByBmBEBnBoBnBxBnByBnBpBnBFBnBGBnBqBoBEBoBpBoBqBoBwBo"
		"BABoBrBpBqBpByBpBrBpBBBpBCBqBKBqBrBqBABqBzBqBBBqBHBqBCBqBFBqBSBrBBBrBA"
		"BrBzBrBJBrBGBrIVBsBBBsIVBsBABsBIBsBKBsJ7BsIwBtIJBtIVBtBBBtIYBtIwBtBABt"
		"KeBtKbBtKdBtJ8BuBvBuBDBuBEBuBwBuBZBuBxBvBwBvBxBvByBvBFBvBzBwBxBwBEBwBy"
		"BwBFBwB#"
		"BwBzBwBABwBYBxBABxByBxBzBxBEBxBZByBzByBGByBAByBSByBIBzBABzBFBzBHBzBVBz"
		"BYBzBWBABHBABGBABSBBBCBBBKBBIYBBBLBCBJBCBIBCBKBCIYBCBHBCKeBCKdBCBLBDBE"
		"BDB#BDB@BEB#BEBZBEB@BEBTBFBIBFBGBFBHBFBSBFBYBFBVBFBXBFBZBFB!"
		"BGBHBGBSBGBIBGBVBGBYBGBWBGBJBHBWBHBKBHBIBHBSBHBJBHBVBHBYBHBLBIKdBIKbBI"
		"BJBIBKBIBSBIBLBIIYBIKeBJKhBJBKBJIYBJKeBJKdBJIJBJKbBJBLBJBSBKIYBKIJBKBL"
		"BKIVBKBSBLKcBLBWBLBVBLBMBLCiBLCfBLBSBLChBLKdBLKeBMCiBMChBMC7BMCaBMBWBM"
		"BXBMBVBMBPBMBNBNC3BNC9BNCaBNC0BNB*BNBQBNC1BNBOBOCOBOCIBOC1BOC0BOB%BOB$"
		"BOC6BOB^BOC5BOBUBOC3BOCLBPKKBPJ@BPJ!"
		"BPCcBPChBPCgBPCiBPC7BPCbBPC8BPCeBPCaBQCWBQCVBQCTBQCSBQCQBQC6BQC9BQC8BQ"
		"CaBQCdBQCeBQC3BQC4BRKOBRKNBRCZBRCXBRCWBRCUBRCcBRKJBRCbBRKKBRCdBRKIBRCT"
		"BRC!BSBVBSBWBSBYBTBWBTB!BTB@BTB*BTC2BTBXBTBYBUB$BUB&BUB%"
		"BUC1BUC0BUC2BVBWBVBYBWBXBXBYBXB!BXBZBXB@BYB!BYBZBZB!BZB#B!B@B@B#B$B%B$"
		"B&B$C1B$C0B$C2B%CLB%C2B%C0B%B^B%C1B^B&B^COB&C2B&C0B&C1B*C0B*C2B*C6B*"
		"CaC0C1C1C2C3C9C3C4C3C6C3CSC3CPC3CRC4CrC4C5C4CSC4CPC4CRC4CLC4CMC4COC5CO"
		"C5CrC5C6C5CSC5CLC5CPC5CRC5CMC6CLC6CMC6COC6CrC7C8C7CaC7CeC7CbC7CdC7ChC8"
		"C9C8CeC8CbC8CdC8ChC9CaC9CeC9CdC9CbCaChCaCiCbKJCbCcCbCeCbKKCcCUCcCdCcCT"
		"CcCWCcKJCcKKCdCTCdCeCdCWCeKKCeCWCfKiCfCgCfCiCfJ!CfJ@"
		"CfJZCfKdCfKcCgKKCgChCgJ!CgJ@CgJZCgKdCgJ#ChCiChJ@CiJZCiKdCiKcCjK#"
		"CjCZCjCYCjC!"
		"CjCkCjKXCjCUCjCtCjCTCjCVCjKYCjCwCjClCkClCkCwCkKXCkCzCkCZCkCYCkK#"
		"CkCmCkC!CkCtCkK@ClD4ClCmClCzClCwClKXClK@ClK#ClCCClC%ClC@ClL6CmK@"
		"CmCBCmC%CmC@CmC$CmCCCmL6CmC^"
		"CmD1CmD0CmD2CmCzCmD4CnCICnCJCnCHCnCNCnCMCnCOCnCoCnCrCnCuCoD8CoCICoCpCo"
		"CuCoCxCpCuCpCqCpCxCpD5CpD8CqDdCqCGCqCBCqCxCqD5CqD6CqD8CqCACqDgCqDcCqD9"
		"CqDfCrCJCrCHCrCvCrCMCrCRCrCLCrCNCrCSCrCQCrCsCrCuCrCICsCQCsCRCsCPCsCtCs"
		"CvCsCVCsCWCsCUCsCuCsCwCsCMCtCvCtCVCtCUCtCWCtCwCtCZCtC!"
		"CtCQCtCPCtCYCtCRCuCSCuCICuCvCuCxCuCyCuCRCuCQCuCMCuCNCvCBCvCwCvCyCvCQCv"
		"CRCvCxCvCzCvCPCvCVCwCYCwCQCwCzCwCyCwCVCwCUCwCWCwCZCxD8CxCyCxCACxCGCxCB"
		"CxDdCxDgCxD5CyCzCyCBCyCDCyCGCyCACyCCCyD4CyDdCzCCCzCDCzCBCzD1CzD4CzC%"
		"CADdCADgCACGCACBCAD9CAD5CAD6CADcCADaCACFCBCDCBCGCBCCCBD1CBDgCCC@"
		"CCCGCCD1CCD4CCCDCCD0CCC^CCC*CCC%CCC$"
		"CCCECDD4CDD3CDD1CDCECDCGCDDUCDDRCDDTCEDtCEC*"
		"CECFCEDUCEDTCEDNCEDRCEDOCED3CEDrCEDQCED2CEDqCED4CFDQCFDNCFDPCFDeCFDpCF"
		"DfCFDdCFDJCFDKCFCGCFDMCGDdCGDeCGDgCGDJCHCICHCKCHCNCHCOCHCMCICJCICNCICO"
		"CICMCJCKCKCNCKCOCKCMCLCMCLCOCLCSCLCRCLCPCMCRCMCPCMCNCMCSCNCPCNCOCNCSCN"
		"CRCPCQCPCSCPCVCPCWCQCRCQCVCQCWCQCUCRCSCSCWCTCUCTCWCTC!"
		"CTCZCTCXCTKYCTKNCUCVCUC!CUCZCUCXCVCWCVC!CVCZCVCXCXKQCXCYCXC!"
		"CXKYCXKNCXKXCXKOCXKMCXKJCYK#CYCZCYKYCYKXCYKNCYKOCYKMCYKJCZC!CZKXCZK#C!"
		"KYC!KNC!KOC!KMC@L9C@C#C@C%C@L6C@L5C@LcC@D1C@DVC@D4C@DWC@K@C#L9C#K@C#"
		"DSC#D3C#C$C#C^C#C&C#L5C#D0C#L6C#DVC#DYC#LcC#D2C#DWC$DXC$D3C$C%C$C^C$C&"
		"C$D0C$L5C$D2C$DYC$D1C$DVC$LcC%K@C%C^C%D0C%C&C%L6C%D1C%D2C%D4C%DYC^L6C^"
		"L5C^C&C^D0C^D2C^DYC^D1C^DVC^D3C^DXC&LcC&L6C&DrC&DhC&C*C&DRC&DYC&DSC&"
		"DUC&DVC&DXC&L5C*DrC*D0C*DRC*DSC*DUC*D2C*DYC*D1C*D3C*DVC*"
		"DXD0DRD0D2D0DUD0D1D0DSD0D3D1D2D1D4D1DUD1DRD1DTD2D3D2DUD2DRD2DTD3D4D3DU"
		"D3DRD3DTD4L6D5DdD5D6D5D8D5DcD5D9D5DbD5DgD5DfD6D7D6DcD6D9D6DbD6DgD6DfD6"
		"DdD7DHD7D8D7DcD7D9D7DbD7DgD7DfD7DdD8DID9DQD9DaD9DcD9DMD9DJD9DLD9DfD9Dg"
		"D9DeDaDQDaDbDaDMDaDJDaDLDaDFDaDIDaDfDaDGDaDgDaDeDbDQDbDlDbDcDbDMDbDJDb"
		"DLDbDFDbDIDbDGDcDFDcDIDcDfDcDGDcDgDcDeDdDeDdDgDeDfDeDJDeDMDeDKDeDPDeDp"
		"DfDPDfDgDfDJDfDMDfDKDgDJDgDMDgDKDhLgDhDjDhDXDhDWDhDYDhDiDhLbDhDSDhDrDh"
		"DRDhDTDhLcDhDuDiLaDiDjDiDuDiLbDiDxDiDXDiDWDiLgDiDkDiDYDiDrDiLfDjE2DjDk"
		"DjDxDjDuDjLbDjLfDjLgDjDADjD#DjDZDjLsDkLfDkD%DkDBDkDzDkD#DkDZDkD@"
		"DkDADkLsDkD$DkD*DkD&"
		"DkE0DkDxDkE2DlDsDlDGDlDHDlDFDlDLDlDKDlDMDlDmDlDpDmE6DmDHDmDnDmDsDmDvDn"
		"DsDnDoDnDvDnE3DnE6DoEbDoDEDoDzDoDvDoE3DoE4DoE6DoDyDoEeDoEaDoE7DoEdDpDH"
		"DpDtDpDKDpDPDpDJDpDLDpDQDpDODpDqDpDsDpDGDpDFDqDODqDPDqDNDqDrDqDtDqDTDq"
		"DUDqDSDqDsDqDuDrDWDrDPDrDNDrDtDrDTDrDSDrDUDrDuDrDXDrDYDrDODsDHDsDtDsDv"
		"DsDwDsDPDsDODsDKDsDLDtDzDtDuDtDwDtDODtDPDtDvDtDxDtDNDuDWDuDODuDxDuDwDu"
		"DTDuDSDuDUDuDXDvE6DvDwDvDyDvDEDvDzDvEbDvEeDvE3DwEcDwDxDwDzDwDBDwDEDwDy"
		"DwDADwE2DwEbDxDADxDBDxDzDxD*DxE2DxD#"
		"DyDBDyEbDyEeDyDEDyDzDyE7DyE3DyE4DyEaDyE8DyDDDzDBDzDEDzDADzD*"
		"DzEeDADZDADEDAD*DAE2DADBDAD&DAD$DAD^DAD#DAD@DADCDBE2DBE1DBD*"
		"DBDCDBDEDBD#DCELDCD%DCE1DCE0DCE2DCDDDCEnDCD^DCD&"
		"DDEnDDE8DDEFDDEGDDEIDDEcDDEdDDEbDDDEDEEbDEEcDEEeDEEFDFDGDFDIDFDLDFDMDF"
		"DKDGDHDGDLDGDMDGDKDHDIDIDLDIDMDIDKDJDPDJDNDJDKDJDMDJDQDKDQDKDPDKDNDKDL"
		"DLDQDLDPDLDNDLDMDNDODNDQDNDTDNDUDODPDODTDODUDODSDPDQDQDUDRDSDRDUDRDYDR"
		"DXDRDVDSDTDSDYDSDXDSDVDTDUDTDYDTDXDTDVDVL6DVDWDVDYDVLcDVLbDVL5DWLgDWDX"
		"DWLcDWLbDWL5DWL6DXLgDXDYDXLbDYL5DYLcDZD!DZD#DZLsDZLrDZLtDZD*"
		"DZE2DZLuDZLfDZLCD!LfD!D@D!D$D!D%D!LrD!D&D!LsD!LtD!E0D!LuD!E1D!END@LuD@"
		"D#D@D$D@D%D@D&D@LrD@LtD@E0D@D*D@E1D#LfD#D$D#D&D#D%D#LsD#D*D#E0D#E2D#"
		"END$LsD$EOD$D%D$D&D$E0D$D*D$E1D$END$LtD$LrD%LsD%D^D%END%EQD%LtD%EOD%"
		"LrD^D&D^E0D^D*D^E1D^EQD^END^EPD^LtD^EmD&E0D&D*D&E1D&EQD&EPD*E0D*"
		"E2E0E1E0EQE0ENE1E2E1EQE1ENE1EPE2LsE3EbE3E4E3E6E3EaE3E7E3E9E3EeE3EdE4E5"
		"E4EaE4E7E4E9E4EeE4EdE4EbE5E6E5EaE5E7E5E9E5EeE5EdE5EbE7E8E7EaE7EdE7EeE7"
		"EcE7EIE7EFE7EHE8E9E8EdE8EIE8EeE8EcE8EHE8EFE8EfE9EfE9EaE9EIE9EHE9EFEaEd"
		"EaEeEaEcEbEcEbEeEbEFEbEGEcEdEcEFEcEIEcEGEcEfEdEeEdEIEdEFEdEHEeEIEfEpEf"
		"EgEfEHEfEGEfEIEfEjEgEGEgEhEgEpEgEqEgEnEgEsEgELEgEMEgEKEhEUEhETEhEtEhEw"
		"EhEuEhELEhEKEhEMEhEiEiMIEiMAEiMcEiM3EiF4EiF1EiETEiESEiEUEiMrEiEoEiEtEi"
		"EuEjEsEjEpEjErEjExEkEEEkECEkEBEkEqEkEwEkErEkEvEkEpEkEtEkExEkEyElE!"
		"ElEXElEqElEyElEzElEEElExElEDElEBEmMcEmM3EmLCEmETEmEoEmEJEmEPEmEKEmEOEm"
		"EMEmEQEmLLEmLUEnEHEnEMEnEGEnEJEnELEnEFEoELEoEJEoEREoEUEoM3EoMcEoLUEoEK"
		"EpEqEpEsEpEHEqErEqExEqEAEqEyErEsErExErEAErEyEsExEsEAEsEyEtEuEtEwEtF4Et"
		"ETEtEUEuF4"
		"EuEYEuEvEuEBEuECEuEEEuEXEvEYEvEwEvEBEvECEvEEEvEXEvF4EwEBEwECEwEEEwEXEx"
		"EyExEAEyEEEyEDEyEzEzEAEBECEBEEEBEXECEDECEXECE!EDEEEDEXEDE!EEE!"
		"EFEGEFEIEGEHEHEIEJEKEJEMEJEPEJEOEJEQEKETEKELEKEPEKEOEKEQELEMELEUEMEPEM"
		"EQENLuENEOENEQENLCENLtENLLEOLMEOEPEOLCEOLLEOLtEPEQEPLLEQLtERESEREUERMc"
		"ERM3ERMdERM4ERLUESETESMcESM3ESM4ESMAESLUETEUETMsETMAEUM3EUM4EULUEVE@"
		"EVEYEVF3EVF4EVE#EVE%EVEWEVEXEVF0EVEZEWF3EWF0EWE#EWE^EWF2EWE@"
		"EWF5EWNiEWNrEXE@EXEYEXE!EYE%EYE@EYEZEYF3EYF4EZE%EZE!EZE$EZE@E@F5E@E#E@"
		"F3E@E%E@F0E@F4E#F0E#F5E#E$E#F3E#E*E#F8E#E^E$E%E$F8E$F5E$F0E%F8E^F3E^"
		"NrE^NiE^E&E^NjE^NHE^F2E^NPE^F0E^FhE&NiE&FeE&FhE&NPE&FgE&NGE&NXE&FfE&"
		"FqE&NQE&NHE*FpE*F6E*F5E*FdE*FeE*FcE*F0E*FfE*FnE*FoE*"
		"FbF0NiF0F5F0F6F0F3F0FdF0FcF1MIF1MRF1MAF1F4F1M!F1MJF1MrF1M@"
		"F1F2F2NsF2N1F2NiF2F3F2M!"
		"F2N2F2NjF2NrF2MRF3NrF3F4F3NiF4MIF4MAF5F6F5FdF5FeF5FnF6FlF6FdF6FcF6FbF6"
		"FnF6FmF6FfF6FoF7F8F7F9F7FaF7FiF7FjF7FkF7FbF7FsF8F9F8FiF9FaF9FiF9FjF9Fr"
		"F9FkF9FtF9FAF9FlFaFjFaFiFaFkFaFsFaFlFaFtFaFBFaFCFbFcFbFlFbFkFbFmFbFtFb"
		"FuFbFsFbFvFbFoFcFmFcFlFcFnFcFkFcFoFcFvFcFtFcFwFcFjFdFFFdFeFdFnFdFmFdFo"
		"FdFfFdFvFdFuFdFwFdFxFeFFFeFfFeFnFeFgFeFmFeFvFeFuFeFhFeFlFfNXFfFgFfFpFf"
		"FoFfFhFfFxFfFqFfFwFfFvFgFhFgFqFgNXFgFoFgFxFgFzFgNPFgNGFgO7FhFIFhFqFhNX"
		"FhNPFhFzFhNYFhFyFhNGFhN*"
		"FiFjFiFrFiFsFiFkFiFAFiFCFiFlFjFDFjFuFjFmFjFkFjFsFjFrFjFtFjFlFjFBFjFAFj"
		"FCFkFDFkFBFkFuFkFlFkFtFkFsFkFmFkFCFkFrFlFrFlFmFlFtFlFuFlFsFlFCFlFBFmFs"
		"FmFvFmFtFmFwFmFDFnFoFnFvFnFuFnFwFnFpFnFFFnFxFoFEFoFpFoFwFoFvFoFxFoFGFo"
		"FyFoFHFpFxFpFwFpFyFpFHFpFvFpFzFpFGFpFJFqO8FqNGFqFzFqN&FqFyFqO7FqFJFqN*"
		"FqFxFqFIFqNPFrFsFrFAFrFBFrFtFrFKFsFEFsFtFsFBFsFCFsFDFtFVFtFSFtFEFtFAFt"
		"FCFtFBFtFDFtFKFuFvFuFEFuFLFuFCFuFBFvFFFvFEFvFMFvFDFwFPFwFxFwFGFwFHFwFy"
		"FwFNFwFMFxFYFxFNFxFJFxFyFxFHFxFGFxFzFxFOFyOgFyN&"
		"FyFZFyFFFyFzFyFHFyFJFyO7FyFGFyFQFzOxFzOhFzNYFzFZFzFPFzFJFzO7FzFIFzN&"
		"FzFQFzO8FAFBFAFCFAFKFAFDFAF@"
		"FBFEFBFCFBFKFBFDFBFUFCFLFCFDFCFKFCFEFCFTFDFEFDFKFDFTFEFLFEFKFEFWFEFOFF"
		"FGFFFMFFFNFFFHFFFOFFFXFFFIFGFVFGFHFGFNFGFMFGFIFGFWFGFXFHFZFHFYFHFMFHFI"
		"FHFOFHFPFHFJFHFXFIOyFIFJFIFOFIOgFIO7FIN&FIOhFJOGFJOxFJN&"
		"FJFQFJOgFJFPFJO7FJFZFJOhFKGZFKG8FKG1FKG0FKF*FKFTFKFUFKF#FKF$FKF%"
		"FLG2FLG0FLF!FLFMFLFUFLFWFLFNFLFTFLF$FLF#FLF^FLFOFMF^"
		"FMFYFMFNFMFVFMFOFMFUFMFXFMF%FNG5FNG1FNFZFNFOFNFWFNFXFNFPFNFYFNF%"
		"FOG5FOG4FOFVFOFPFOFXFOFYFOFQFOF&FOFWFPOGFPF^FPFVFPFQFPFXFPFZFPF&"
		"FPOgFPOxFQOHFQFZFQFYFQOgFQOxFQOhFQOGFQF&FRF!FRF@FRF#FRF*FRF$"
		"FRGaFRG2FSFTFSF@FSF#FSFUFSF$FSF*FSFVFSF%"
		"FSGaFSFWFTG8FTG1FTG0FTFWFTFUFTF#FTF$FTFVFTF*FTF!FTF%FUGaFUG2FUF!"
		"FUFVFUF$FUF#FUF%FUFWFUG0FUF*FVFWFVF^FVG2FVF&FWGbFWG5FWF%FWF^FWF&"
		"FWG1FXOGFXGfFXGeFXFYFXF&FXF^FXG4FXG5FYF&FYF^"
		"FYG6FYOGFYOgFZOWFZOxFZOGFZG5FZOgFZGeF!F@F!F#F!F$F!F%F@F#F@F*F@F$F@GaF@"
		"F%F@GgF@GbF#GgF#G2F#F$F#F*F#F%F#G8F#GaF$GiF$G7F$F%F$G0F$F*F$G1F$GaF%"
		"GjF%G1F%G2F%GaF^F&F^G3F^GdF^GcF^GkF&OxF&G4F&G5F&GeF&G6F&GdF*GaF*GbF*"
		"GhF*"
		"GYG0G1G0GaG0GbG0G2G0GiG0GjG0G3G1GhG1G7G1G2G1G3G1G9G1GdG1G4G2G3G2GbG2G4"
		"G2GeG2GfG3GNG3G4G3GdG3GcG3GeG3GkG3GfG3GiG3GnG4GeG4GdG4GcG4GmG4GjG4OGG5"
		"GeG5GdG5GlG5GnG5OGG5OVG6GeG6OxG6OHG6OWG6O%"
		"G6GdG6OIG6OXG6PHG6PGG7G8G7GgG7GhG7G9G7G@G7G!"
		"G7GaG7GbG8G9G8GaG8GbG8GiG8GoG8GcG9GaG9GhG9GbG9GiG9GYG9GrG9GcG9GdG9GAGa"
		"G@"
		"GaGgGaGdGaGbGaGcGaGhGbGcGbGjGbGdGbGlGcGdGcGiGcGkGcGoGcGTGdGkGdGjGdGoGe"
		"GfGeGnGeGTGeGSGeO%GfO^GfGmGfGlGfGnGfGTGfGkGfO%GfGQGfGSGgGzGgGhGgG@"
		"GgGYGgGrGgG!GhGYGhGrGhGZGhGoGhGqGiG@"
		"GiGjGiGoGiGkGiGlGiGrGiGYGjGkGjGoGjGlGjGDGjGmGjGAGkGNGkGnGkGlGkGmGkGTGk"
		"GDGkGSGlGmGlGTGlGnGlGQGlGSGlGAGlO%GmO^GmOWGmGAGmGnGmGTGmGQGmGSGmO%"
		"GmOVGmPHGmPGGnO%GnGQGnOVGnGTGnGRGnO^"
		"GoGCGoGDGoGpGoGrGoGYGpGNGpGzGpGLGpGIGpGCGpGDGpGKGpGqGpGwGqGwGqGzGqGZGq"
		"G!"
		"GqGrGqGYGqGLGrGYGrGZGsPeGsPfGsGBGsGtGsGvGsPvGsPuGsGCGtPmGtGFGtGUGtGVGt"
		"PuGtPvGtGXGtGuGtPeGuGXGuGWGuGUGuGFGuGEGuGvGuGJGvGFGvGJGvGIGvGCGvGBGvGK"
		"GvGEGwGIGwGGGwGxGwGzGwGLGwGKGwG!"
		"GwGMGxGGGxGyGxGPGxGMGxGHGxGKGxGLGxGEGyGzGyGPGyGMGyGOGyGHGyGEGzG!"
		"GzGZGzG@"
		"GAGSGAGBGAGDGAGRGAGTGAPGGBPeGBGCGBGIGBGSGBPfGBGJGCGIGCGDGCGJGCGLGDGIGD"
		"GSGDGLGEGFGEGHGEGKGEGJGEGWGFGGGFGXGFGWGFGJGGGNGGGMGGGHGGGOGGGWGHG!"
		"GHGMGHGNGHGPGHGKGHGJGIGJGIGLGJGWGJGPGJGKGKGWGKGPGKGLGKGMGLGPGLGNGMGNGM"
		"GPGNGOGOGPGOGWGQOXGQOVGQGRGQGTGQPHGQPGGQO%"
		"GQPEGRPfGRP6GRGSGRPGGRPHGRPFGRO%GSPFGSGTGSPGGTO%"
		"GTOVGTPHGUGVGUGXGUPuGUPtGUPmGUPCGVPDGVPBGVGWGVPuGVPvGVPtGVPmGVPlGWPBGW"
		"GXGXPuGXPtGYGZGYG@GZG!G!G@G#HpG#HoG#H9G#G$G#G^G#HuG#H8G#H6G#HhG#HeG$"
		"HpG$HhG$HeG$H8G$H9G$HgG$G%G$H2G%HuG%HhG%G^G%H2G%H5G%HvG%HwG^HuG^HvG^"
		"HpG&HoG&H7G&G*G&H1G&H8G&H6G&HfG&HeG&HnG*I6G*I3G*I1G*HnG*HqG*HrG*HtG*"
		"I7G*I4G*I0G*H0H0H*H0H^"
		"H0HfH0H1H0HtH0HsH0HqH0HbH0I3H0HaH0I2H1HnH1HfH1HeH1H8H1H7H1HgH1HaH2HiH2"
		"HeH2H3H2H5H2HhH2HgH3HaH3H4H3HlH3HiH3HdH3HgH3HhH4HiH4H5H4HlH5HvH6HoH6H7"
		"H6H9H6HnH6HpH7HpH7H8H7HeH7HoH7HfH7HnH8HeH8H9H8HfH8HhH9HnH9HeH9HoH9HhH9"
		"HpHaHbHaHdHaHgHaHfHaHsHbI3HbI0HbH!HbHHHbHcHbH%HbH*HbH^HbHtHbHsHcH&HcH%"
		"HcH@HcHjHcHiHcHdHcHkHcH!HcH*HcH$HdH!"
		"HdHiHdHjHdHlHdHgHdHfHeHfHeHhHfHgHgHhHgHiHiHjHiHlHjH&HjH%HjHWHjHkHjH!"
		"HjH$HjH@HjH*HkHZHkHXHkHWHkHlHkH$HkH!HkH#"
		"HmHnHmHpHmHuHmHxHmI4HnHoHoHpHqHrHqHtHqI7HqI4HqI6HqPIHrPIHrI6HrI0HrHsHr"
		"I3HrI1HrI7HrI4HsH*HsH^HsH%HsI3HsHtHsI0HsI2HsHIHtHIHtH%HtH^"
		"HuHvHuHxHvHwHwHxHyQiHyHAHyI6HyI5HyI7HyPRHyP!"
		"HyHzHyI1HyHIHyPIHyI0HyI2HyHLHzQsHzI5HzHAHzQ1HzHLHzP!"
		"HzQiHzHOHzQ2HzI6HzPRHzQjHzQrHAQiHAHBHAHOHAQ1HAQrHAHLHAQ2HAHRHAIbHAI8HB"
		"QPHBQHHBQjHBIjHBIdHBHQHBIbHBI8HBQrHBIaHBHRHBQiHBIcHBIgHBIfHBIhHBHOHCHJ"
		"HCHXHCHYHCHWHCH#HCH@HCH$"
		"HCHDHCHGHDHYHDHXHDHEHDHJHDHMHEHFHEHMHEIkHEInHEHJHEIlHFIsHFIqHFHQHFIkHF"
		"IlHFInHFHPHFIvHFIrHFIoHFIuHFHMHGHKHGH@HGH&HGH!HGH#HGH*HGH^"
		"HGHHHGHJHGHXHGHWHGHYHHH!HHH^HHH&HHH%HHHIHHHKHHI2HHI3HHI1HHHJHHHLHIH&"
		"HIHKHII2HII1HII3HIHLHII6HII7HIH^HIH%HII5HJHKHJHMHJHNHJH&HJH^HJH@HJH#"
		"HKHQHKHLHKHNHKH^HKH&HKHMHKHOHKH%"
		"HKI2HLQiHLI5HLHOHLHNHLI2HLI1HLI3HLI6HMInHMHNHMHPHMHVHMHQHMIsHMIvHMIkHN"
		"ItHNHOHNHQHNHSHNHVHNHPHNHRHNIjHNIsHOQrHOHRHOHSHOHQHOIgHOIjHOQiHPIsHPIv"
		"HPHVHPHQHPIoHPIkHPIlHPIrHPIpHPHUHQHSHQHVHQHRHQIgHQIvHRQrHRI8HRIgHRIjHR"
		"HSHRIfHRIcHRIeHRIbHRIaHRHTHSIjHSIiHSIgHSHTHSHVHSI@HSI!"
		"HSJIHTITHTILHTIeHTIdHTI@HTI!HTJIHTJHHTIyHTIiHTIhHTIjHUI!"
		"HUIHHUIqHUIpHUHVHUISHUITHUJpHUJqHUIyHUItHUIuHUIsHVIsHVItHVIvHVISHVITHV"
		"JpHWHXHWHZHWH#HWH$HWH@HXHYHXH#HXH$HXH@HYHZHZH#HZH$HZH@H!H@H!H$H!H*H!H&"
		"H!H%";

	const char coded_edges4[] =
		"H@H%H@H#H@H*H@H&H#H$H#H*H#H&H%H^H%H*H%I2H%I3H^H&H^I2H^I3H^I1H&H*H*"
		"I3I0I1I0I3I0I7I0I6I0I4I0PII0P!"
		"I1I2I1I7I1I6I1I4I1PII2I3I2I7I2I6I2I4I4I5I4I7I4PII4P!"
		"I4PJI4Q1I5I6I5PRI5PII5P!I5PJI5P@I5Q3I6I7I6PRI6P!I6P@"
		"I7PII7PJI8I9I8IbI8QGI8QPI8QQI8IgI8IjI8Q^I8QtI9Q^"
		"I9QII9IMI9IiI9IaI9QGI9IcI9QPI9IdI9IfI9IZI9QHI9QQI9ILI9IhIaIiIaIbIaIcIa"
		"IdIaIfIaQPIaIZIaIhIaIgIaILIaQQIbIcIbIfIbIdIbIgIbIhIbIjIbQsIbQiIcQQIcQH"
		"IcIdIcIfIcIZIcIhIcIgIcQPIcIiIcQGIdQ^IdQQIdQHIdIeIdILIdIZIdQPIdQGIdI@"
		"IeIfIeILIeIhIeIZIeIgIeIiIeI@IeQ^IeIyIfILIfIhIfIgIfIiIfI@"
		"IfQGIgQrIgIhIgIjIgILIhIiIhI@IhILIiIjIiI@IiILIiJIIjI@"
		"IjJIIkIlIkInIkIrIkIoIkIqIkIvIkIuIkIsIlImIlIrIlIoIlIqIlIvIlIuIlIsImInIm"
		"IrImIoImIqImIvImIuImIRImIsIoIpIoIrIoIHIoIuIoIvIoItIoISIpIqIpIHIpIRIpIu"
		"IpIvIpItIpISIqIrIqIHIqIRIqISIrIRIrIuIrIvIrItIsItIsIvIsISIsJpItIuItISIt"
		"IHItJpItIRIuIvIuISIuIHIvIHIwJcIwJbIwJ4IwJ3IwJ7IwJ8IwIQIwIVIwIPIwIUIxQ%"
		"IxQ!IxJmIxJdIxJhIxJiIxI#IxI&IxI$"
		"IxJ0IxJeIxJlIyJHIyJFIyJsIyJqIyIAIyJpIyJIIyISIyI@IyITIyI!"
		"IzKqIzKpIzKnIzKiIzKfIzJ%IzJUIzJSIzJDIzJBIzI^"
		"IzICIzKjIzKmIzKkIzJCIzJTIzIXIzI%IzIWIAJMIAJLIAJJIAJwIAJuIAI!"
		"IAITIAISIAJqIAJHIAJpIAJIIAJrIAJGIAJtIBJPIBJNIBJMIBJKIBJkIBJjIBJ6IBJ5IB"
		"JuIBJxIBJLIBJQIBJtIBJvIBJyIBJAICJQICJPICJOICJzICJxICI^"
		"ICJBICJUICJCICJTICJEICJRICJyIDJ4IDJ1IDJ3IDJsIDIPIDITIDJpIDJ8IEJdIEJeIE"
		"JgIEJFIEI!IEI$"
		"IEJIIEJhIFJbIFJaIFJcIFJDIFIUIFIWIFJCIFJ7IGJmIGJlIGJnIGJSIGI^"
		"IGJ0IGJTIGJiIHIRIHISIIIQIIIRIJIVIJIYIJKeIKKlIKJ$"
		"IKKfIKKiIKKgIKIXIKIYIKKbIKKcIKKeILQPILIZILI@IMQ%IMQ^IMIZ"
		"IMI#IMQPIMQGIMI&INQ@INI&INI*INR6INKrINR7INR4INR5IOKnIOKqIOKoIOI%IOI*"
		"IOKuIOKtIOKrIPJ4IPIQIPJ8IPJ3IPJ5IQJ8ISITISJpITJ4ITJpITJsITJ1IUJbIUIVIU"
		"J7IUJcIUJ6IUJ9IVJ7IWJbIWIXIWJCIWJDIWJaIWKmIWKjIWKlIXKiIXKgIXJ$"
		"IXJCIXKmIXKjIXKlIXKfIYJZIYKeIYKbIYKiIYKhIYKfIZQ%IZQIIZQGIZQPIZQ^I!JdI!"
		"I@I!JII!JFI!JgI@JII#Q!I#I&I#I$I#JhI#Q%I#Q^I#JiI#JkI$JhI$JeI$JkI$JfI$"
		"JiI$JdI%KmI%KkI%J^I%J%I%I^I%JTI%KqI%KpI%KnI%KjI^KmI^J%I^JmI^JTI^JSI^"
		"JnI^KjI^KkI&R5I&R0I&Q!I&J0I&JiI&Q@I&JhI&JjI*RgI*R5I*KqI*KrI*KuI*R6I*Q@"
		"I*KnI*KoJ0Q!"
		"J0JiJ0JlJ0JjJ0JoJ0JhJ0JmJ1JsJ1JrJ1J2J1J4J1JwJ1JtJ2JsJ2JwJ2JtJ2JvJ2J5J2"
		"JrJ2JqJ2J3J3JvJ3J8J3J4J3J5J3JwJ4JsJ4J8J5JAJ5JxJ5JvJ5JuJ5JwJ5J6J5J8J6Jv"
		"J6JuJ6JAJ6JxJ6JzJ6J9J6J7J6JcJ7JAJ7JbJ7JcJ7J8J8JvJ9JDJ9JzJ9JyJ9JAJ9JEJ9"
		"JBJ9JaJ9JcJaJDJaJEJaJbJaJzJaJyJbJcJbJDJcJAJcJzJdJeJdJgJdJFJdJhJeJKJeJh"
		"JeJfJeJkJeJJJfJFJfJhJfJJJfJKJfJMJfJkJfJGJfJHJfJgJgJFJgJGJgJJJgJMJhJKJh"
		"JiJhJkJhQ%JiQ%"
		"JiJmJiJlJiJjJiJoJiJNJjJLJjJKJjJNJjJOJjJQJjJoJjJkJjJlJkJQJkJNJkJKJkJJJk"
		"JLJlJNJlJmJlJoJlJOJmJnJmJSJnJSJnJRJnJoJnJOJnJPJoJSJoJOJoJNJoJPJoJRJoJU"
		"JpJqJpJsJqJuJqJrJqJtJqJwJrJsJrJwJrJtJrJvJsJwJtJuJtJwJuJLJuJvJuJxJvJwJx"
		"JQJxJyJxJAJxJBJyJzJyJBJyJEJzJAJzJEJzJBJAJEJBJCJBJEJBKmJBKjJBKlJCJDJCKm"
		"JCKjJCKlJDJEJDKjJFJGJFJIJFJJJGJHJGJJJGJMJGJKJHJLJHJIJHJMJHJJJJJKJJJMJK"
		"JLJLJMJNJRJNJOJNJQJOJPJOJRJOJUJPJQJPJUJPJRJQJUJRJSJRJUJSKkJSJTJSKjJTKm"
		"JTJ%JTJUJTKjJTKkJUKkJUKjJVROJVJXJVKtJVKsJVKuJVRfJVRoJVJWJVKoJVJ^"
		"JVR6JVKnJVKpJVK0JWRYJWKsJWJXJWRxJWK0JWRoJWROJWK3JWRyJWKtJWRfJWRPJWRXJX"
		"KGJXROJXJYJXK3JXRxJXRXJXK0JXRyJXK6JXKyJXKvJYSdJYS5JYRPJYKGJYKAJYK7JYK5"
		"JYKyJYKvJYRXJYKxJYK6JYROJYKzJYKDJYKCJYKEJYK3JZJ@"
		"JZKcJZKdJZKbJZKhJZKgJZKiJZJ!JZJ$JZJ&J!KKJ!KiJ!J@J!J&J!K1J!KcJ!J#J!KdJ!"
		"J$J!KbJ@KPJ@J#J@K1J@KHJ@KKJ@J&J@KIJ#KaJ#K5J#KHJ#KIJ#KKJ#K4J#KSJ#KOJ#"
		"KLJ#KRJ#K1J#KPJ#KNJ$KdJ$KbJ$J*J$KgJ$KlJ$KfJ$KhJ$KmJ$KkJ$J%J$J&J$KcJ%"
		"KoJ%KfJ%K0J%J&J%KkJ%KlJ%KjJ%J^J%J*J%KpJ%KqJ^J*J^KpJ^KoJ^KqJ^K0J^KtJ^"
		"KuJ^KkJ^KjJ^KsJ&KfJ&J*J&K1J&K2J&KlJ&KkJ&KgJ&KhJ*K5J*K0J*K2J*KkJ*KlJ*"
		"K1J*K3J*KjJ*"
		"KpK0ROK0KsK0K3K0K2K0KpK0KoK0KqK0KtK1KKK1K2K1K4K1KaK1K5K1KPK1KSK1KHK2KQ"
		"K2KFK2K3K2K5K2K7K2KaK2K4K2K6K2KGK2KPK3RXK3KBK3K6K3K7K3K5K3KDK3KGK3ROK4"
		"K7K4KPK4KSK4KaK4K5K4KLK4KHK4KIK4KOK4KMK4K9K5K7K5KaK5K6K5KDK5KSK6RXK6Kv"
		"K6KaK6KDK6KGK6K7K6KCK6KzK6KBK6KyK6KxK6K8K7KGK7KFK7KDK7K8K7KaK7L2K8L1K8"
		"KWK8KVK8KTK8L2K8KFK8KEK8KGK8K9K8KBK8KCK8KAK9KTK9KVK9KQK9KRK9KPK9KaK9KM"
		"K9KLK9KWKaKPKaKQKaKSKaKVKbKcKbKeKbKhKbKiKbKgKcKdKcKhKcKiKcKgKdKeKeKhKe"
		"KiKeKgKfKgKfKiKfKmKfKlKfKjKgKhKgKmKgKlKgKjKhKiKhKmKhKlKhKjKjKqKjKpKjKk"
		"KjKmKkKqKkKoKkKlKkKpKlKmKmKqKnKoKnKqKnKuKnKtKnKrKnR6KnRoKoKpKoKuKoKtKo"
		"KrKoR6KpKqKpKuKpKtKpKrKrKsKrKuKrR6KrRoKrR7KrQ@"
		"KrRxKsKtKsRfKsR6KsRoKsR7KsRpKsQ@KtKuKtRfKtRoKtRpKuR6KuR7KuQ@"
		"KvKwKvKyKvS4KvSdKvSeKvKDKvKGKvSlKvRZKwSmKwS6KwKFKwKxKwS4KwKzKwSdKwKAKw"
		"KCKwS5KwSeKwK*KwKEKwSlKxSmKxSlKxKyKxKzKxKAKxKCKxSdKxKEKxKDKxSeKxK*"
		"KxKFKyROKyKzKyKCKyKAKyKDKyKEKyKGKyK*"
		"KyRYKzSmKzSlKzSeKzS5KzKAKzKCKzKEKzK*"
		"KzKDKzSdKzKFKzS4KASmKASeKAS5KAKBKAK*KASdKAS4KAL2KASlKBKCKBKEKBK*"
		"KBKDKBKFKBL2KBSlKBSmKCS5KCKEKCKDKCKFKCL2KCS4KDRXKDKEKDKGKEKFKEL2KEK*"
		"KFKGKFL2KFK*"
		"KGRXKHKIKHKKKHKOKHKLKHKNKHKSKHKRKHKPKIKJKIKOKIKLKIKNKIKSKIKRKIKPKJKKKJ"
		"KOKJKLKJKNKJKSKJKRKJKYKJKPKLKMKLKOKLKRKLKSKLKQKLKVKMKNKMKYKMKRKMKVKMKS"
		"KMKQKNKOKNKYKNKVKOKYKOKRKOKSKOKQKPKQKPKSKQKRKQKVKQKYKQKWKRKSKRKVKRKYKT"
		"K&KTKZKTKWKTL1KTL2KTK!KTK#KTKUKTKVKUSqKUSoKUL1KUK&KUK!KUK$"
		"KUL0KUKZKUL3KUSpKUKWKVKZKVKWKVKYKWK#KWKZKWKXKWL1KWL2KXKZKXK#KXKYKXK@"
		"KZL3KZK!KZL1KZK#KZK&KZL2K!K&K!L3K!K@K!L1K!K^K!L6K!K$K@K&K@K#K@L6K@L3K$"
		"SsK$SrK$LmK$SpK$K%K$L0K$K&K$SoK$L1K$LlK$SqK$SmK%SsK%SrK%LlK%SpK%LoK%K^"
		"K%SqK%L4K%SoK%L0K%LmK^L4K^L3K^K&K^LoK^L9K^LlK^L7K^LaK&L0K&L3K&L4K&L1K*"
		"SpK*SoK*SnK*S6K*S4K*L0K*SlK*SdK*L2K*"
		"SmL0SrL0SqL0SmL0L1L0SoL0SpL0SlL0SnL1SqL1SpL1SoL1L3L1L2L3L4L3LoL3L6L4Ll"
		"L4L9L4LoL4L7L4LaL4L5L4LcL5LbL5L9L5L6L5LcL6LcL7LdL7LaL7LnL7LoL7LeL7LgL7"
		"L8L7L9L8LnL8LkL8LeL8LhL8LmL8LdL8LpL8LaL8L9L9LlL9LaL9LcL9LdL9LoLaLgLaLd"
		"LaLbLaLnLaLoLbLdLbLgLbLcLdLpLdLhLdLeLdLnLdLgLdLkLdLoLeLmLeLkLeLpLeLfLe"
		"LnLeLjLeLsLeLhLfLgLfLsLfLpLhLlLhLiLhLmLhLkLhLBLhLnLhLKLhLTLiLmLiLBLiLA"
		"LiLzLiLKLiLyLiLjLiLTLiLSLiLILjLJLjLqLjLpLjLxLjLyLjLwLjLkLjLzLjLHLjLILj"
		"LvLkLmLkLpLkLqLkLnLkLxLkLwLlLoLlLmLlSpLlSoLmLnLmLBLnLoLnLpLoSqLpLqLpLx"
		"LpLyLpLHLqLFLqLxLqLwLqLvLqLHLqLGLqLzLqLILrLvLrLsLrLtLrLuLrLCLrLDLsLtLt"
		"LULtLuLtLCLtLDLtLLLtLELuLVLuLDLuLCLuLELuLMLuLFLuLNLvLwLvLFLvLELvLGLvLN"
		"LvLOLvLMLvLPLvLILwLGLwLFLwLHLwLELwLILwLPLwLNLwLQLwLDLxLZLxLALxLyLxLHLx"
		"LGLxLILxLzLxLPLxLOLxLQLxLRLySzLyLZLyLzLyLHLyLALyLGLyLPLyLOLyLBLyLFLzSz"
		"LzLALzLJLzLILzLBLzLRLzLKLzLQLzLPLASALALBLALKLALILALRLALTLASzLAL@LAL$"
		"LBSMLBSLLBSBLBLKLBSzLBLTLBLSLBL$LBL#"
		"LBLQLCLDLCLLLCLMLCLELCLULCLWLDLXLDLOLDLGLDLELDLMLDLLLDLNLDLFLDLVLDLULD"
		"LWLELXLELVLELOLELFLELNLELMLELGLELWLELLLFLGLFLNLFLOLFLMLFLWLFLVLGLPLGLN"
		"LGLQLGLXLHLILHLPLHLOLHLQLHLJLHLZLHLRLISzLILYLILJLILQLILPLILRLIL!"
		"LILSLIL@LJSALJLRLJLQLJLSLJL@LJLPLJLTLJL!LJL$"
		"LKSMLKSLLKSBLKLTLKLSLKSALKL$LKLRLKL#LKL@LKM2LLL%"
		"LLLMLLLULLLVLLLNLMLYLMLNLMLVLMLWLMLXLNM7LNM4LNLYLNLULNLWLNLVLNLXLNL%"
		"LOLPLOLYLOL^LOLWLOLVLPL#LPLZLPLYLPL&LPLXLQM1LQLRLQL!LQL@LQLSLQL*LQL&"
		"LRSALRSzLRMaLRL*LRL$LRLSLRL@LRL!LRLTLRM0LSSCLSSBLSSzLSMbLSLZLSLTLSL@"
		"LSL$LSSALSL!LSM2LTSMLTSLLTSCLTSBLTMbLTL$LTSALTL#"
		"LTM2LTSzLTM1LUMdLULVLULWLUL%LULXLVLYLVLWLVL%LVLXLVM6LWL^LWLXLWL%"
		"LWLYLWM5LXLYLXL%LXM5LYL^LYL%LYM8LYM0LZL!LZL&LZL*LZL@LZM0LZM9LZL#L!M7L!"
		"L@L!L*L!L&L!L#L!M8L!M9L@SzL@MqL@MbL@MaL@L&L@L#L@M0L@M1L@L$L@M9L#SCL#"
		"SBL#L$L#M0L#SAL#SzL#M8L$SLL$SDL$SCL$SBL$SzL$M2L$SAL$M1L$MbL$M9L%MsL%"
		"MlL%MkL%MjL%M5L%M6L%MeL%MfL%MgL^MmL^MkL^L&L^M6L^M8L^L*L^M5L^MfL^MeL^"
		"MhL^M0L&MhL&MaL&L*L&M7L&M0L&M6L&M9L&MgL*MpL*MlL*MbL*M0L*M8L*M9L*M1L*"
		"MaL*"
		"MgM0MpM0MoM0M7M0M1M0M9M0MaM0M2M0MiM0M8M1SDM1SCM1SzM1M7M1M2M1M9M1MbM1Mi"
		"M1SAM1MhM2SLM2SJM2SFM2SBM2SzM2MbM2MaM2SAM2SCM2MiM2SDM3McM3MdM3MeM3MjM3"
		"MfM3MuM4M5M4MdM4MeM4M6M4MfM4MjM4M7M4MgM5MsM5MlM5MkM5M8M5M6M5MeM5MfM5M7"
		"M5MjM5McM5MgM6MuM6MmM6McM6M7M6MfM6MeM6MgM6M8M6MkM6MjM7M8M7MhM7MmM7MiM8"
		"MvM8MsM8MqM8MpM8MgM8MhM8MiM8MlM9SDM9MzM9MyM9MaM9MiM9MhM9MoM9MpMaMiMaMh"
		"MaMqMaSDMaSAMbSFMbSEMbSBMbSDMbMpMbSCMbSAMbMyMcMdMcMeMcMfMcMgMdMeMdMjMd"
		"MfMdMuMdMgMdMAMdMvMeMmMeMfMeMjMeMgMeMsMeMuMfMrMfMgMfMkMfMjMfMlMfMuMgMy"
		"MgMlMgMmMgMuMhMiMhMnMhMxMhMwMhMEMiSDMiMoMiMpMiMyMiMqMiMxMjMuMjMvMjMBMj"
		"MJMkMlMkMuMkMvMkMmMkMCMkMDMkMnMlMmMlMnMlMtMlMxMlMoMmMnMmMvMmMoMmMyMmMz"
		"MnMoMnMxMnMwMnMyMnMEMnMzMnMCMnMNMoMyMoMxMoMwMoMGMoMDMoMCMpSEMpMyMpMxMp"
		"MFMpMHMpMQMpSCMqSDMqMyMqMxMqSCMqSEMqSNMqSFMqSAMrMsMrMAMrMBMrMtMrMIMrMu"
		"MrMRMrMSMrMTMsMtMsMuMsMvMsMCMsMLMsMwMtMuMtMBMtMvMtMCMtMJMtMKMtMwMtMSMt"
		"MUMuMAMuMvMuMwMuMBMvMwMvMDMvMxMvMKMvMFMwMxMwMCMwMEMwMLMwMNMwMKMwMOMxML"
		"MxMEMxMDMxMNMxMOMySDMyMzMyMHMyMPMzMGMzMFMzMHMzMPMzMEMzMQMzMNMzMZMAMBMA"
		"MIMAMJMAMRMAMTMAM!MBMJMBMSMBMKMBMRMBMTMBM@"
		"MBMLMCMFMCMDMCMLMCMKMCMMMCMEMCMVMCMUMCMTMDMGMDMEMDMMMDMLMDMFMDMWMDMKMD"
		"MUMEMHMEMFMEMNMEMOMEMGMEMYMFSNMFMGMFMNMFMPMFMHMFMQMFMWMFMZMGSEMGMHMGMP"
		"MGMOMGMQMGMZMGMXMGSNMHSFMHSEMHMQMHMPMHMZMHMOMHSDMHN0MIMJMIMRMIMSMIM!"
		"MIMTMIM#MIN1MIMUMJMSMJMRMJMTMJM@MJMUMJM#MJN2MKM@MKMLMKMUMKMTMKMVMKM#"
		"MKM$MLMVMLMUMLMWMLMTMLMXMLM%MLM#MMN6MMMNMMMWMMMVMMMXMMMOMMM%MMM$MMM^"
		"MMM&MMMPMNN6MNMQMNMOMNMWMNMPMNMVMNM%MNM$MOSNMOM^MOMPMOMYMOMXMOMQMOM&"
		"MOMZMPSOMPSNMPSEMPMQMPMZMPMXMPM&MPN0MQSPMQSEMQN9MQMZMQSNMQN0MQM*"
		"MQNaMRMSMRM!MRM@MRMTMRN1MRN3MSN4MSM$MSMVMSMTMSM@MSM!MSM#"
		"MSMUMSN2MSN1MSN3MTN4MTN2MTM$MTM!MTMUMTM#MTM@MTMVMTN3MUN2MUN1MUMVMUM#"
		"MUM$MUM@MUN3MVM%MVM#MVM^MVN4MWMXMWM%MWM$MWM^MWMYMWN6MWM&MXN5MXMYMXM^"
		"MXM%MXM&MXN7MXM*MXN8MYSOMYM&MYM^MYM*MYN8MYM%"
		"MYN0MYN7MYNaMZSPMZNhMZN0MZM*MZSOMZNaMZM&MZN9M!M@M!N1M!N2M!M#M!NbM@N5M@"
		"M#M@N2M@N3M@N4M#NmM#NjM#N5M#N1M#N3M#N2M#N4M#NbM$M%M$N5M$NcM$N3M$N2M%"
		"N9M%N6M%N5M%NdM%N4"
		"M^NgM^M&M^N7M^N8M^M*M^NeM^NdM&SOM&SNM&NpM&NeM&NaM&M*M&N8M&N7M&N0M&NfM*"
		"SQM*SNM*NqM*N6M*N0M*N8M*NaM*SOM*N7M*"
		"NhN0SQN0SPN0SEN0NqN0NaN0SON0N9N0NhN0SNN0NgN1N2N1N3N1NbN1N4N1NsN2N5N2N3"
		"N2NbN2N4N2NlN3NcN3N4N3NbN3N5N3NkN4N5N4NbN4NkN5NcN5NbN5NnN5NfN6N7N6NdN6"
		"NeN6N8N6NfN6NoN6N9N7NmN7N8N7NeN7NdN7N9N7NnN7NoN8NFN8NqN8NpN8NdN8N9N8Nf"
		"N8NgN8NaN8NoN9SQN9SPN9NaN9NfN9SON9SNN9NnNaSRNaSQNaSPNaSNNaNhNaSONaNgNa"
		"NqNaNoNbNHNbNANbNzNbNyNbNkNbNlNbNtNbNuNbNvNcNBNcNzNcNrNcNdNcNlNcNnNcNe"
		"NcNkNcNuNcNtNcNwNcNfNdNwNdNpNdNeNdNmNdNfNdNlNdNoNdNvNeNENeNANeNqNeNfNe"
		"NnNeNoNeNgNeNpNeNvNfNENfNDNfNmNfNgNfNoNfNpNfNhNfNxNfNnNgSRNgSQNgNmNgNh"
		"NgNoNgNqNgNxNgSONgNwNhSPNhSNNhNqNhNpNhSONhSQNhNxNhSRNiNrNiNsNiNtNiNyNi"
		"NuNiNJNjNkNjNsNjNtNjNlNjNuNjNyNjNmNjNvNjNJNkNHNkNANkNzNkNnNkNlNkNtNkNu"
		"NkNmNkNyNkNrNkNvNlNJNlNBNlNrNlNmNlNuNlNtNlNvNlNnNlNzNlNyNmNnNmNwNmNBNm"
		"NxNnNKNnNFNnNENnNvNnNwNnNxNnNANoSRNoNONoNNNoNpNoNxNoNwNoNDNoNENpNxNpNw"
		"NpNFNpSRNpSONqSPNqSRNqNENqSQNqSONqNNNrNsNrNtNrNuNrNvNsNtNsNyNsNuNsNJNs"
		"NvNsNPNsNKNtNBNtNuNtNyNtNvNtNHNtNJNuNvNuNzNuNyNuNANuNJNvNANvNBNvNJNwNx"
		"NwNCNwNMNwNLNwNTNxSRNxNDNxNENxNNNxNFNxNMNyNJNyNKNyNQNyNYNzNANzNJNzNKNz"
		"NBNzNRNzNSNzNCNANBNANCNANINANMNANDNBNCNBNKNBNDNBNNNBNONCNDNCNMNCNLNCNN"
		"NCNTNCNONCNRNCN#NDNNNDNMNDNLNDNVNDNSNDNRNESSNENNNENMNENUNENWNEN^"
		"NESQNFSRNFNNNFTeNFNMNFSQNFSSNFT4NFSTNGNHNGNPNGNQNGNINGNXNGNJNGN&NGN*"
		"NHNINHNJNHNKNHNRNHN!NHNLNINJNINQNINKNINRNINYNINZNINLNIN*"
		"NIO1NJNKNJNLNJNQNKNLNKNSNKNMNKNZNKNUNLNMNLNRNLNTNLN!NLN#NLNZNLN$NMN!"
		"NMNTNMNSNMN#NMN$NNSRNNNONNNWNNN%NNTeNOT4NONVNONUNONWNON%NONTNON^NON#"
		"NOO6NPNQNPNXNPNYNPN&NPO0NPO7NQNYNQN*NQNZNQN&NQO0NQO8NRNUNRNSNRN!"
		"NRNZNRN@NRNTNRO2NRO1NRO0NSNVNSNTNSN@NSN!NSNUNSO3NSNZNSO1NTNWNTNUNTN#"
		"NTN$NTNVNTO5NUNVNUN#NUN%NUNWNUN^NUO3NUO6NVSSNVNWNVN%NVN$NVN^"
		"NVTeNVO6NVO4NWSSNWN^NWN%NWO6NWN$NWSRNWOfNXNYNXN&NXN*NXO7NXO0NXO9NYN*"
		"NYN&NYO0NYO8NYO1NYO9NZN!NZO1NZO0NZO2NZO9NZOaN!O2N!O1N!O3N!O0N!O4N!ObN!"
		"O9N@OlN@N#N@O3N@O2N@O4N@N$N@ObN@OaN@OcN@OdN@N%N#OlN#N^N#N$N#O3N#N%N#"
		"O2N#ObN#OaN$OcN$N%N$O5N$O4N$N^N$OdN$O6N%T5N%OfN%N^N%O6N%TeN%O4N%OdN^"
		"SSN^OoN^TeN^O6N^OfN^OeN^OpN&N*N&O7N&O8N&O0N&OgN*OjN*OiN*OgN*OaN*O2N*"
		"O0N*O8N*O7N*O9N*O1N*"
		"OhO0OjO0OhO0OaO0O1O0O9O0O8O0O2O0OiO1O2O1O9O1OaO1O8O1OiO2ObO2O9O2OcO2Oj"
		"O3O4O3ObO3OaO3OcO3O5O3OlO3OdO4OkO4O5O4OcO4ObO4OdO4OmO4OeO4OnO5OdO5OcO5"
		"OeO5OnO5ObO5OfO5OmO5OpO6OfO6T5O6OeO6TeO6OpO6OdO6OoO7OqO7O8O7OgO7OhO7O9"
		"O8OkO8O9O8OhO8OiO8OjO9OBO9OyO9OkO9OgO9OiO9OhO9OjO9OqOaObOaOkOaOrOaOiOa"
		"OhObOlObOkObOsObOjOcOdOcOmOcOnOcOeOcOtOcOsOdOtOdOeOdOnOdOmOdOfOdOuOeTe"
		"OeOlOeOfOeOnOeOpOeT5OeTdOeOmOfOpOfT5OfOoOfTdOfOwOfOvOgOHOgOqOgOjOgOhOg"
		"OiOhOkOhOiOhOqOhOjOiOrOiOjOiOqOiOkOiOzOjOkOjOqOjOzOkOrOkOqOkOCOkOuOlOm"
		"OlOsOlOtOlOnOlOuOlODOlOoOmOBOmOnOmOtOmOsOmOoOmOCOmODOnOUOnOEOnOsOnOoOn"
		"OuOnOvOnOpOnODOoOpOoOuOoTdOoT5OoTDOoS@OpTdOpOwOpOvOpTDOpT5OpS@"
		"OpOFOqOPOqOOOqONOqOzOqOAOqOIOqOJOqOKOrOQOrOOOrOsOrOAOrOCOrOtOrOzOrOJOr"
		"OIOrOLOrOuOsOLOsOtOsOBOsOuOsOAOsODOsOKOtOTOtOPOtOuOtOCOtODOtOvOtOEOtOK"
		"OuOTOuOSOuOBOuOvOuODOuOEOuOwOuOMOuOCOvOLOvOBOvOwOvODOvOFOvS@"
		"OvTdOvOMOvTcOwThOwOMOwOFOwS@"
		"OwTdOwOEOwTcOwTDOwTCOwTiOxOGOxOHOxOIOxONOxOJOyOzOyOHOyOIOyOAOyOJOyONOy"
		"OBOyOKOyOYOyOCOzOWOzOPOzOOOzOCOzOAOzOIOzOJOzOBOzONOzOGOzOKOAOYOAOQOAOB"
		"OAOJOAOIOAOKOAOCOAOOOAONOBOCOBOLOBOQOBOMOCOZOCOKOCOLOCOMOCOPODO$ODO#"
		"ODOEODOMODOLODOSODOTOEOMOES@OETiOEOLOEOUOETdOETDOFS@"
		"OFTiOFThOFTCOFTDOFTdOFOTOFTcOFT6OGOHOGOIOGOJOHOZOHOIOHONOHOJOHOYOHOKOH"
		"O%OIOQOIOJOIONOIOKOIOWOIOYOJO&OJOKOJOOOJONOJOPOJOYOKO*"
		"OKOPOKOQOKOYOLOMOLOROLO@OLO!OLP0OMT6OMOSOMOTOMO#OMOUOMO@ONOYONOZONO^"
		"ONPEOOOPOOOYOOOZOOOQOOO&OOO*OOOROPOQOPOROPOXOPO@OPOSOQOROQOZOQOSOQO#"
		"OQO$OROSORO@ORO!ORO#ORP0ORO$ORO&ORP3OSPwOSO#OSO@OSO!OSP2OSO*OSTiOTO#"
		"OTTiOTO@OTP1OTP3OTTCOTS@OUT6OUTiOUO#OUTlOUS@OUO@OUTDOUTdOUTcOVOWOVO%"
		"OVO^OVOXOVPHOVPGOVOYOWOXOWOYOWOZOWO&OWP4OWO!OXOYOXO^OXOZOXO&"
		"OXPEOXP7OXO!OYO@OYOZOYO!OYO^OZO!OZO*OZO@OZP1O!O@O!O&O!P0O!P4O!PzO@P0O@"
		"O*O@P4O#O$O#P3O#PzO#PyO#TlO$P2O$P1O$P3O$PzO$TlO$P0O$PwO$PyO%O^O%PHO%"
		"PEO%P7O%PGO^PEO^P7O^PFO^P4O&O*O&P4O&P0O&P1O&P7O&PEO*P0O*P4O*P1O*PjO*"
		"P2O*"
		"PgP0PwP0PgP0P3P0P1P0P2P0PzP0PjP0PyP1P2P1PzP1P3P1PwP1PyP1PgP1TlP2P3P2Pz"
		"P2PwP2PyP2TlP2T6P2PgP3P9P3TlP3PwP3PzP3T6P3PxP3TiP4T7P4PiP4PjP4P5P4P7P4"
		"PEP5PrP5PoP5PiP5PjP5PqP5P6P5PcP6PrP6PcP6PfP6PFP6PGP6P7P6PEP7PEP7PFP8Ph"
		"P8P9P8PbP8PiP8PgP8PpP8PoP8PxP9PAP9PBP9PDP9PaP9PlP9PxPaPDPaPCPaPAPaPlPa"
		"PkPaPbPaPpPbPlPbPpPbPoPbPiPbPhPbPqPbPkPcPoPcPdPcPfPcPrPcPqPcPGPcPsPdPm"
		"PdPePdPvPdPsPdPnPdPqPdPrPdPkPePfPePvPePsPePuPfPGPfPFPgPyPgPhPgPjPgPxPg"
		"PzPhPiPhPoPhPyPhPpPhPxPiPoPiPjPiPpPiPrPjPxPjPoPjPyPjPrPkPlPkPnPkPqPkPp"
		"PkPCPlPmPlPDPlPCPlPpPmPtPmPsPmPnPmPuPmPCPnPsPnPtPnPvPnPqPnPpPoPpPoPrPp"
		"PCPpPqPqPCPqPvPqPrPqPsPrPtPsPtPsPvPtPuPuPvPwPxPwPzPxPyPxT6PyPzPzT6PzTl"
		"PAPBPAPDPBPCPCPDPEPFPEPHPFPGPGPHPIQ1PIPJPIPRPIPSPIP!PIPTPIP#"
		"PJPSPJPRPJPTPJP@PJPUPJP#PJQ2PJQ3PKPLPKPUPKPTPKPVPKP#PKP$PKP@PKP%"
		"PKPXPKQ3PKP!PKQ2PLPVPLPUPLPWPLPTPLPXPLP%PLP#PLP^PLPSPLQ4PLP@"
		"PMPQPMPNPMPWPMPVPMPXPMPOPMP%PMP$PMP^PMP&PMPPPMQ6PMP*"
		"PNPOPNPWPNPPPNPVPNP%PNP$PNPQPNPUPNQ6PNQ5PNPTPOPPPOPYPOPXPOPQPOP&"
		"POPZPOP^POP%POQ8POQ7PPPQPPPZPPPXPPP&PPQ0PPQ8PPQaPPP%PPQ7PQPZPQQ0PQP*"
		"PQQaPQQ9PQP^PQQ8PQQhPRPUPRPSPRP!PRP@PRPTPRQ1PRQ3PSP$PSPVPSPTPSP@PSP!"
		"PSP#PSPUPSQ2PSQ1PSQ3PSQ4PTQ4PTQ2PTP$PTPUPTP#PTP@PTPVPTQ3PTP!PUPVPUP#"
		"PUP$PUP@PUQ3PUQ2PVP%PVP#PVP^PVQ4PVP@PWP*PWPXPWP%PWP$PWP^PWPYPWQ6PWP&"
		"PWQ8PWQdPXQFPXQ5PXPYPXP^PXP%PXP&PXQ7PXP*PXQ8PYQhPYP&PYP^PYP*PYQ8PYP%"
		"PYQ0PYQ7PYQaPYQfPZQqPZQ0PZP*PZQaPZP&PZQ9PZQ8PZQhPZP^PZQ7P!QbP!P@P!Q1P!"
		"Q2P!P#P@Q5P@P#P@Q2P@Q3P@Q4P#QmP#QjP#Q5P#Q1P#Q3P#Q2P#Q4P#QbP$P%P$Q5P$"
		"QcP$Q3P$Q2P%Q9P%Q6P%Q5P%QdP%Q4P^QqP^QgP^Q0P^P&P^Q7P^Q8P^P*P^QeP^QdP&"
		"QpP&QeP&QaP&P*P&Q8P&Q7P&Q0P&QfP*QqP*Q6P*Q0P*Q8P*QaP*Q7P*"
		"QhQ0QqQ0QaQ0Q9Q0QhQ0QgQ0QfQ1QsQ1Q2Q1Q3Q1QbQ1Q4Q2Q5Q2Q3Q2QbQ2Q4Q2QlQ3QF"
		"Q3QcQ3Q4Q3QbQ3Q5Q3QkQ4Q5Q4QbQ4QkQ5QcQ5QbQ5QnQ5QfQ6QpQ6Q7Q6QdQ6QeQ6Q8Q6"
		"QfQ6QoQ6Q9Q7QOQ7QEQ7QmQ7Q8Q7QeQ7QdQ7Q9Q7QnQ7QoQ8QFQ8QEQ8QqQ8QpQ8QdQ8Q9"
		"Q8QfQ8QgQ8QaQ8QoQ9QaQ9QfQ9QnQ9QwQ9QvQaQhQaQgQaQqQaQoQbQAQbQzQbQyQbQkQb"
		"QlQbQtQbQuQbQvQcQBQcQzQcQyQcQdQcQlQcQnQcQeQcQkQcQuQcQtQcQwQcQfQdQwQdQq"
		"QdQpQdQgQdQeQdQmQdQfQdQlQdQoQdQvQeQEQeQAQeQqQeQhQeQfQeQnQeQoQeQgQeQpQe"
		"QvQfQEQfQDQfQmQfQgQfQoQfQpQfQhQfQxQfQnQgQmQgQhQgQoQgQqQgQxQgQwQhQqQhQp"
		"QhQxQiQrQiQsQiQtQiQyQiQuQjQkQjQsQjQtQjQlQjQuQjQyQjQmQjQvQjQJQkQHQkQAQk"
		"QzQkQnQkQlQkQtQkQuQkQmQkQyQkQrQkQvQlQJQlQBQlQrQlQmQlQuQlQtQlQvQlQnQlQz"
		"QlQyQmQFQmQnQmQwQmQBQmQxQnQMQnQKQnQFQnQEQnQvQnQwQnQxQnQAQoQWQoQOQoQNQo"
		"QpQoQxQoQwQoQDQoQEQpQxQpQwQpQFQpQMQqQEQqQNQrQsQrQtQrQuQsQKQsQtQsQyQsQu"
		"QsQJQsQvQsQPQtQBQtQuQtQyQtQvQtQHQtQJQuQRQuQvQuQzQuQyQuQAQuQJQvQSQvQNQv"
		"QAQvQBQvQJQwQxQwQCQwQMQwQLQwQTQxQTQxQDQxQEQxQNQxQFQxQMQyQJQyQKQyQQQzQA"
		"QzQJQzQKQzQBQzQRQzQSQzQCQAQQQAQBQAQCQAQIQAQMQAQDQBQWQBQCQBQKQBQDQBQNQB"
		"QOQCQDQCQMQCQLQCQNQCQTQCQOQCQRQCQWQDQNQDQMQDQLQDQVQDQSQDQRQEQNQEQMQEQU"
		"QEQWQFQNQFQMQGQ%QGQHQGQPQGQQQGQIQGQ^QGQJQHQLQHQIQHQJQHQKQHQ^"
		"QHQRQIQJQIQQQIQKQIQRQIQLQIQ^"
		"QIQMQJQMQJQKQJQLQJQQQKQLQKQSQKQMQKQUQLQMQLQRQLQTQLQ#QLQ$"
		"QMQTQMQSQNUVQNQOQNQWQNQXQOQVQOQUQOQWQOQTQOQXQOUUQOQ$QPQ%QPQQQPQ^QQQ#"
		"QQQ^QRQSQRQTQRQ#QRQUQRQVQSR1QSQWQSQTQSQUQSQ#QSQVQSQ$QSQYQTUSQTR1QTQ$"
		"QTQYQTQUQTQVQTQ#QTQWQUUSQUQ$QUQVQUQWQUQXQUQ#"
		"QUUVQUQYQVQWQVQXQVUVQVUUQVQYQVQ#QWQXQWQYQXR1QXQ$QXQ#"
		"QXUUQXUVQXUTQXQYQXQ&QXUtQYUTQYQ$QYQ#QYQ&"
		"QYR1QYUtQZUZQZRbQZR0QZR3QZRdQZReQZR2QZUIQZRcQZVrQZUJQZQ*QZUsQ!R0Q!R5Q!"
		"R1Q!Q$Q!Q%Q!Q@Q@R8Q@R0Q@R4Q@R6Q@R7Q@R5Q#Q$Q$R1Q$Q%Q%R1Q%R0Q%Q^Q&UVQ&Q*"
		"Q&UtQ&UsQ&UkQ&UUQ&UJQ&UTQ&R1Q*UsQ*UtQ*R2Q*UJQ*R3Q*UIQ*UGQ*"
		"R0R0R1R0R5R0R2R0UsR1R5R2R3R2RcR2RdR2UsR2ReR2RmR2R5R3RcR3RbR3RdR3RaR3Rm"
		"R3ReR3R9R3RkR4R5R4R8R4R7R4R9R4RiR4RaR4RgR4RjR5R7R5R6R6RxR6R7R6RfR6RgR6"
		"RoR6RhR7RgR7RfR7RhR7RpR7RiR7RqR8R9R8RiR8RhR8RjR8RqR8Rr"
		"R8RpR8RsR8RlR8RzR9RjR9RiR9RkR9RhR9RlR9RsR9RqR9RtR9RgR9RARaRbRaRkRaRjRa"
		"RlRaRcRaRsRaRrRaRtRaRuRaRdRaRCRbRcRbRkRbRdRbRjRbRsRbRrRbReRbRiRbRCRcRd"
		"RcRmRcRlRcReRcRuRcRnRcRtRcRsRdReRdRnRdVrRdRlRdRuRdRwRdVoReRFReRnReVrRe"
		"VqReVoReRwReRvReUIReRGRfRiRfRgRfRoRfRpRfRhRfRxRfRzRgRARgRrRgRjRgRhRgRp"
		"RgRoRgRqRgRiRgRyRgRxRgRzRhRARhRyRhRrRhRiRhRqRhRpRhRjRhRzRhRoRiRjRiRqRi"
		"RrRiRpRiRzRiRyRjRsRjRqRjRtRjRARjRpRkRlRkRsRkRrRkRtRkRmRkRCRkRuRkRERkRJ"
		"RlRBRlRmRlRtRlRsRlRuRlRDRlRvRlRERmRuRmRtRmRvRmRERmRsRmRwRmRDRmRGRnRNRn"
		"RwRnVqRnRvRnVpRnRGRnRuRnRFRnRERnUYRoRHRoRpRoRxRoRyRoRqRpRBRpRqRpRyRpRz"
		"RpRARqRSRqRPRqRBRqRxRqRzRqRyRqRARqRHRrRsRrRBRrRIRrRzRrRyRsRFRsRCRsRBRs"
		"RJRsRARtRMRtRuRtRDRtRERtRvRtRKRtRJRuRVRuRKRuRGRuRvRuRERuRDRuRwRuRLRvRW"
		"RvRCRvRwRvRERvRGRvRDRvRNRvVqRwU%RwUYRwRWRwRMRwRGRwRFRwVqRwU*"
		"RwRNRwVpRwVrRxRYRxRyRxRzRxRHRxRARyRBRyRzRyRHRyRARyRRRzRIRzRARzRHRzRBRz"
		"RQRARBRARHRARQRBRIRBRHRBRTRBRLRCRDRCRJRCRKRCRERCRLRCRURCRFRDScRDRSRDRE"
		"RDRKRDRJRDRFRDRTRDRURES3RERWRERVRERJRERFRERLRERMRERGRERURFRGRFRLRFU*"
		"RFVqRFUYRGVpRGU&RGU%RGRNRGU*RGRMRGRWRGUYRGVqRHR&RHR^RHR%"
		"RHRQRHRRRHRZRHR!RHR@RIR*RIR^RIR%RIRJRIRRRIRTRIRKRIRQRIR!RIRZRIR#"
		"RIRLRJR#RJRVRJRKRJRSRJRLRJRRRJRURJR@RKS2RKR&"
		"RKRWRKRLRKRTRKRURKRMRKRVRKR@RLSyRLSbRLS2RLS1RLRSRLRMRLRURLRVRLRNRLR$"
		"RLRTRMR#RMRSRMRNRMRURMRWRMU*RMU&RMR$RNU*RNRWRNRVRNU&RNUYRNR$"
		"RORXRORYRORZROR%ROR!RPRQRPRYRPRZRPRRRPR!RPR%RPRSRPR@RPS7RQS5RQR&RQR^"
		"RQRTRQRRRQRZRQR!RQRSRQR%RQRXRQR@RRS7RRR*RRRXRRRSRRR!RRRZRRR@RRRTRRR^"
		"RRR%RSRTRSR#RSR*RSR$RTS8RTS3RTS2RTR@RTR#RTR$RTR&RUScRUSbRURVRUR$RUR#"
		"RUS1RUS2RVR$RVU*RVR#RVS3RVVfRWSiRWS9RWU&RWU*RWS2RWVfRWSbRXRYRXRZRXR!"
		"RYRZRYR%RYR!RYS7RYR@RYSdRZR*RZR!RZR%RZR@RZS5RZS7R!SfR!R@R!R^R!R%R!R&R!"
		"S7R@SgR@SbR@R&R@R*R@S7R#R$R#S0R#SaR#S9R#ShR$ShR$S1R$S2R$SbR$S3R$SaR%"
		"SlR%S7R%S8R%SeR^R&R^S7R^S8R^R*R^SfR^SgR^S0R&SeR&R*R&S0R&S6R&SaR&S1R*"
		"S0R*S8R*S1R*SbR*"
		"ScS0SkS0S1S0SaS0S9S0SbS0ShS0ScS0SfS0SxS1SyS1SbS1SaS1S9S1SjS1SgS1SfS2Sx"
		"S2SbS2SaS2SiS2SkS2VeS2SyS3U&S3VfS3VeS3SbS3U^S3SaS3U*"
		"S3V7S4SmS4S9S4S5S4SdS4SeS4S6S4SlS4S7S5S9S5S6S5S7S5S8S5SfS6SnS6S7S6SeS6"
		"S8S6SfS6SlS6S9S6SaS7SdS7SaS7S8S7S9S7SeS8S9S8SgS8SaS8SxS8SiS9SaS9SfS9Sh"
		"SaSySaSwSaShSaSgSaSxSbSwSbScSbSkScSjScSiScSkScShScSyScSxScSwSdSnSdSmSd"
		"SeSdSlSeSoSeSnSeSmSeSlSfSwSfSvSfSnSfSjSfSgSfShSfSxSfSiSfSlSgVlSgSvSgSn"
		"SgShSgSxSgSiSgSjSgSkSgSwShVlShSyShSwShSvShSuShStShSsShSiShSxShSjShSkSi"
		"VlSiSvSiSuSiStSiSjSiSkSiSxSiSySiSwSjVnSjVlSjVdSjSuSjStSjSkSjSySjSxSjVe"
		"SjSwSjVfSkVnSkVlSkV6SkSxSkSuSkStSkSySkVeSkVfSkVdSkSwSlSrSlSpSlSoSlSmSl"
		"SnSmSsSmSrSmSqSmSnSmSoSmSpSnSuSnStSnSsSnSqSnSoSnSvSnSrSnSxSnSpSnSwSoSu"
		"SoStSoSpSoSrSoSqSoSvSoSsSpSuSpStSpSqSpSrSpSvSpSsSqSrSqSsSqSvSqSuSqStSr"
		"SvSrSsSrSuSrSwSrStSrSxSsSuSsStSsSvSsSwSsSxSsSyStSuStSvStSwStSyStSxSuSv"
		"SuSwSuSxSuSySvSwSvSxSvSySwSxSwSySxSySyVfSyU^"
		"SzSMSzSLSzSKSzSJSzSASzSBSASMSASLSASKSASJSASFSASDSASBSASCSBSMSBSKSBSISB"
		"SGSBSCSBSJSBSFSBSLSBSDSCSMSCSLSCSKSCSISCSGSCSDSCSFSCSESCSJSDSKSDSJSDSI"
		"SDSGSDSESDSFSDSNSESKSESJSESISESHSESGSESFSESNSESZSESOSESPSFS!"
		"SFSMSFSLSFSHSFSJSFSGSFSZSFSISFSNSFSKSGSMSGSLSGSISGSHSGSJSGSZSGS!SGV^"
		"SGV%SGSKSGSYSGSNSHSMSHS!SHV%"
		"SHSISHSZSHSYSHVDSHSJSHVASHSKSHVCSHSXSHSWSISJSISKSIS!"
		"SISZSISLSISMSISYSIVDSJSKSJSLSJSMSJSZSKSLSKSMSKS!SKSZSLSMSMS!"
		"SNSZSNSOSNSPSOSZSOSPSOSQSPSYSPSQSPSXSPSTSPSZSPSRSQSRSQSTSQSSSQSXSRSXSR"
		"SSSRSTSRTeSRT4SSSXSST4SSTfSSSTSSTeSSS$SSS^SSToSSU5STSWSTSXSTSUSTTfSTS$"
		"STTgSTTUSTT4STTVSTToSTTnSTU5SUSYSUSWSUToSUT8SUSVSUU5SUSXSUS$"
		"SUTmSUVCSUTfSUTgSUWBSUTUSUS^"
		"SVSXSVTmSVWBSVVSSVT8SVVTSVWySVWASVSWSVW1SVVRSVVCSVToSVVKSVVBSWV^SWV$"
		"SWS!SWVCSWVDSWSXSWVTSWVBSWSYSWVSSWT8SWTmSWVQSWToSWWBSXV$SXS!"
		"SXSYSXSZSXVCSXVDSXToSXS$SXTnSYVDSYSZSYS!SYV%SYV^SYVCSYVuSYV$SYVASZS!"
		"SZV%SZV^SZVDS!VUS!V^S!V%S!V#S!VuS!VDS!VAS!VCS@THS@TGS@TzS@TyS@TCS@TDS@"
		"TdS@TiS@TcS@ThS#WoS#TRS#TQS#TJS#TIS#W8S#W9S#TMS#TNS#TpS#TtS#TqS#TvS$"
		"U2S$TXS$S^S$TUS$U5S$TfS$ToS$TgS$TnS$TVS$U4S%UfS%U0S%S*S%T*S%UgS%TkS%"
		"TrS%TjS%TsS%T&S%UhS^U9S^U8S^U6S^T@S^TZS^ToS^TnS^TgS^TfS^TVS^U4S^TUS^"
		"U5S^TWS^U3S^TYS&UcS&UaS&U9S&U7S&TPS&TOS&TBS&TAS&TZS&T#S&U8S&UdS&TYS&T!"
		"S&T$S&T^S*UdS*UcS*UbS*T%S*T#S*TsS*TrS*TkS*TjS*T&S*UhS*T*S*UgS*U1S*UeS*"
		"T$"
		"T0TzT0TwT0TyT0TXT0TcT0TgT0TUT0TDT1TIT1TJT1TLT1U2T1TnT1TqT1U5T1TMT2TGT2"
		"TFT2THT2U0T2ThT2TjT2T*"
		"T2TCT3TRT3TQT3TST3UfT3TsT3TvT3UgT3TNT4TeT4TfT4ToT5TdT5TeT6TiT6TlT7TkT7"
		"TlT7TrT7TbT7TuT8TmT8ToT8VST8TfT8VTT9WAT9TmT9TpT9WBT9WzT9W9T9W0T9W6TaTt"
		"TaTuTaWoTaWpTaW8TaW7TaW9TaTpTbTrTbTuTbTkTbTlTcTzTcTdTcTDTcTyTcTATdTDTf"
		"TgTfTUTfU5TfU4TgTUTgTXTgTwTgTzTgU5ThTGThTiThTCThTHThTBThTETiTCTjTkTjT*"
		"TjU0TjTFTjTGTjUgTkT*"
		"TkUgTkUhTkTrTmWBTmWATmWyTmVSTmToTnToTnU5TnU2TnTLTnTITnTUToU5ToTUToTVTp"
		"WBTpWATpWzTpTqTpTMTpW9TpW0TpW6TpW8TqW6TqTITqTMTqTJTqW9TqW8TqTPTqTKTrTu"
		"TrTsTrUgTrT*TrT&TsUgTsUfTsTSTsTRTsT*"
		"TtWoTtWmTtTuTtTvTtTNTtW8TtWpTtW7TtW9TuWxTuWvTuWoTuWpTuWnTuWgTvW7TvTRTv"
		"TNTvTQTvW8TvW9TvTOTvTTTwTXTwTWTwTxTwTzTwT@TwTYTxTXTxT@TxTYTxT!"
		"TxTATxTWTxTVTxTyTyT!TyTDTyTzTyTATyT@TzTXTzTDTAT^TAT#TAT!TATZTAT@"
		"TATBTATDTBT!TBTZTBT^TBT#TBT%TBTETBTCTBTHTCT^TCTGTCTHTCTDTDT!TEU0TET%"
		"TET$TET^TEU1TET&TETFTETHTFU0TFU1TFTGTFT%TFT$TGTHTGU0THT^THT%"
		"TITJTITLTIU2TITMTJU7TJTMTJTKTJTPTJU6TKU2TKU6TKU7TKU9TKTPTKU3TKU4TKTLTL"
		"U2TLU3TLU6TLU9TMW6TMW0TMU7TMW9TMTNTMTPTMW8TNWpTNUaTNTRTNTQTNW8TNTOTNW9"
		"TOU8TOU7TOUaTOUbTOUdTOTTTOTPTOTQTPUdTPUaTPU7TPU6TPU8TQUaTQTRTQTTTQUbTR"
		"TSTRUfTSUfTSUeTSTTTSUbTSUcTTUfTTUbTTUaTTUcTTUeTTUhTUTVTUTXTUTYTUU5TVTW"
		"TVTYTVT@TVTZTWTXTWT@TWTYTWT!TXT@TYTZTYT@TZU8TZT!TZT#T!T@T#UdT#T$T#T^T#"
		"T&T$T*T$T%T$T&T$U1T%U0T%T^T%U1T%T&T^U1T&T*T&U1T*U0T*"
		"UgU0U1U2U3U2U5U2U6U3U4U3U6U3U9U3U7U4U5U4U9U4U6U4U8U5U9U6U7U6U9U7U8U8U9"
		"UaUeUaUbUaUdUbUfUbUcUbUeUbUhUcUgUcUdUcUhUcUeUdUhUeUfUeUhUfUgUgUhUiUNUi"
		"UMUiUxUiUjUiUlUiUSUiUwUiUuUiUFUiUCUjUNUjUFUjUCUjUwUjUxUjUEUjUkUjUqUkUF"
		"UkUqUkUtUkUTUkUUUkUlUkUSUlUSUlUTUlUNUmVgUmUMUmUvUmUnUmUpUmUwUmUuUmUDUm"
		"UCUmULUnULUnUOUnUPUnVgUnURUnVjUnUoUnV0UoURUoUQUoUOUoUzUoUyUoUpUoUDUpUL"
		"UpUzUpUDUpUCUpUwUpUvUpUEUpUyUqUCUqUrUqUtUqUFUqUEUqUUUqUGUrUsUrUJUrUGUr"
		"UBUrUEUrUFUrUyUsUtUsUJUsUGUsUIUtUVUtUUUtUTUuUMUuUvUuUxUuULUuUNUvUNUvUw"
		"UvUCUvUMUvUDUvULUwUCUwUxUwUDUwUFUxULUxUCUxUMUxUFUxUNUyUzUyUBUyUEUyUDUy"
		"UQUzVgUzUAUzUWUzURUzUQUzUDUAUHUAUGUAUBUAUIUAUWUAUZUAVoUBUGUBUHUBUJUBUE"
		"UBUDUCUDUCUFUDUEUEUFUEUGUGUHUGUJUHUIUHVoUHUZUHUWUHVrUIVpUIUJUIVoUIUZUI"
		"VrUJVoUKULUKUNUKUSUKUVUKVgUKVhULUMUMUNUNUSUOUPUOURUOVjUOVgUOViUOU!"
		"UPUQUPVjUPVgUPViUPV0UPV3UQUWUQURUQVjUQViUQVgUQV0URUWURV3URUXUSUTUSUVUT"
		"UUUUUVUWV2UWV0UWV3UWUXUWUZUWVoUXVdUXVbUXV8UXV2UXV3UXVaUXUYUXU%"
		"UYVoUYVbUYUZUYU%UYU*UYVpUYVqUZVoUZVpU!V1U!U@U!U$U!V2U!V0U!V9U!V8U!VhU@"
		"VkU@VlU@VnU@U#U@V5U@VhU#VnU#VmU#VkU#V5U#V4U#U$U#V9U$VhU$V5U$V9U$V8U$"
		"V2U$V1U$VaU$V4U%VcU%V8U%U^U%U*U%VbU%VaU^V6U^V4U^U&U^VfU^VcU^V7U^VaU^"
		"VbU&VeU&VcU&U*U&"
		"VfV0ViV0V1V0V3V0VhV0VjV1V2V1V8V1ViV1V9V1VhV2V8V2V3V2V9V2VbV3VhV3V8V3Vi"
		"V3VbV3VjV4V5V4V7V4VaV4V9V4VmV5V6V5VnV5VmV5V9V6VdV6VcV6V7V6VeV6VmV7VcV7"
		"VdV7VfV7VaV7V9V8V9V8VbV9VmV9VaVaVmVaVbVaVcVbVdVcVdVcVfVdVeVeVfVgVhVgVj"
		"VhViViVjVkVlVkVnVlVmVmVnVoVpVoVrVpVqVqVrVsVXVsVWVsVHVsVtVsVvVsV#"
		"VsVGVsVEVsVPVsVMVtVXVtVPVtVMVtVGVtVHVtVOVtVuVtVAVuVAVuVDVuV$VuV%"
		"VuVvVuV#VuVPVvV#VvV$"
		"VvVXVwWqVwVWVwVFVwVxVwVzVwVGVwVEVwVNVwVMVwVVVxVVVxVYVxVZVxWqVxV@"
		"VxWtVxVyVxWaVyV@VyV!"
		"VyVYVyVJVyVIVyVzVyVNVzVVVzVJVzVNVzVMVzVGVzVFVzVOVzVIVAVMVAVBVAVDVAVPVA"
		"VOVAV%VAVQVBVIVBVCVBVTVBVQVBVLVBVOVBVPVCVDVCVTVCVQVCVSVCVLVDV%VDV$VDV^"
		"VEVWVEVFVEVHVEVVVEVXVFVXVFVGVFVMVFVWVFVNVFVVVGVMVGVHVGVNVGVPVHVVVHVMVH"
		"VWVHVPVHVXVIVJVIVLVIVOVIVNVIV!VJWqVJVKVJV&VJV@VJV!"
		"VJVNVKVRVKVQVKVLVKVSVKV&"
		"VKW1VKWyVLVQVLVRVLVTVLVOVLVNVMVNVMVPVNVOVOVPVOVQVQVRVQVTVRVSVRWyVRW1VR"
		"V&VRWBVSWzVSVTVSWyVSW1VSWBVTWyVUVVVUVXVUV#VUV^VUWqVUWrVVVWVWVXVXV#"
		"VYVZVYV@VYWtVYWqVYWsVYW2VZV!VZWtVZWqVZWsVZWaVZWdV!V&V!V@"
		"V!WtV!WsV!WqV!WaV@WnV@V&V@WdV@V*V#V$V#V^V$V%V%V^V&WcV&WaV&WdV&V*V&W1V&"
		"WyV*WnV*WlV*WiV*WcV*WdV*WkV*W0V*"
		"W6W0WlW0W1W0W6W0W9W0WzW0WAW1WyW1WzW2WbW2W3W2W5W2WcW2WaW2WjW2WiW2WrW3Wu"
		"W3WvW3WxW3W4W3WfW3WrW4WxW4WwW4WuW4WfW4WeW4W5W4WjW5WrW5WfW5WjW5WiW5WcW5"
		"WbW5WkW5WeW6WiW6W7W6W9W6WlW7WlW7WgW7WeW7W8W7WpW7WmW7WhW7WkW8WoW8WmW8W9"
		"W8WpW9WAW9WzWaWsWaWbWaWdWaWrWaWtWbWcWbWiWbWsWbWjWbWrWcWiWcWdWcWjWcWlWd"
		"WrWdWiWdWsWdWlWdWtWeWfWeWhWeWkWeWjWeWwWfWgWfWxWfWwWfWjWgWnWgWmWgWhWgWo"
		"WgWwWhWmWhWnWhWpWhWkWhWjWiWjWiWlWjWwWjWkWkWwWkWlWkWmWlWnWmWnWmWpWnWoWo"
		"WpWqWrWqWtWrWsWsWtWuWvWuWxWvWwWwWxWyWzWyWBWzWAWAWB";

	using Geometry::Point;
	using Geometry::Vector;
	using std::pair;
	using std::priority_queue;
	using std::string;
	using std::vector;

	typedef pair<double, int> Pdi;
	typedef pair<Point, Point> Segment;
	typedef int VertexID;  // 标记一下哪些值是vertex_list里的编号
	struct Edge
	{
		int v_, next_;
		double w_;
	} edge_list[kMaxEdgeNum];

	vector<Point> vertex_list;
	vector<Segment> hard_edges;
	vector<VertexID> grass_vertices;

	int vertex_list_sz = 0, hard_edges_sz = 0, grass_vertices_sz = 0;
	int area_edges_start[MAP_SZ + 1] = { 0 };
	int edge_list_head[kMaxVertexNum], edge_num = 0;
	VertexID area_point_start[MAP_SZ + 1] = { 0 }, area_grass_start[MAP_SZ + 1];

	double dist[kMaxVertexNum];
	VertexID path_prev[kMaxVertexNum];
	priority_queue<Pdi, vector<Pdi>, std::greater<Pdi>> sssp_heap;

	bool IntersectsBlock(const Point &p, const Point &q);
	VertexID NearestVertex(XYPosition pos);
	void AddBiEdge(VertexID u, VertexID v, double w);
	void ConstructVertexList(void), ConstructGraph(void);
	void SSSP(VertexID s, VertexID t);
}  // namespace Graph

//	<---------------------------- Communication --------------------------->
namespace Communication
{
	struct State
	{
		int sender_vocation;
		double x, y;	//位置
		bool save;		//需要救
		bool medicine;  //需要药
		bool gun;		//需要枪
	} team_state[4];	//存储队友状态

	struct Order
	{
		double x, y;  //位置
		int id;
		bool attack;
		bool pick;
		bool avoid;
	};

	Order OrderDecode(int msg);			  //	Updates order
	int OrderEncode(const Order &order);  //	Encodes a message
	State StateDecode(int msg);
	int StateEncode(const State &state);
	void OrderSend(const Order &order, int target = VOCATION_SZ);  //默认群发
	State GetMyState(void);
	void StateSend(const State &state, VOCATION target = VOCATION_SZ);  //默认群发
	void StateReceive(const Sound &radio);
}  // namespace Communication

//	<---------------------------- Attack --------------------------->
namespace Attack
{
	int farthest_range = 1;
	int my_bullet = 0;
	int medcine[2] = { 0, 0 };
	int enemy_amount = 0;
	bool sparta_mode = false;
	const double weapon_dps[10] = { 5, 6.25, 25, 10, 20, 40, 12.5, 15, 10, 5 };  //dps of wepons
	OtherInfo attack_enemy;													   //enemy to attack
	std::vector<ITEM> my_weapon;											   //all weapons

	void UpdateAttackInfo(void);			  //update farthest_range,my_bullet,determine farthest weapon,then determine attack mode
	void DetectEnemy(void);					  //find attack_enemy
	void SmartShoot(const OtherInfo &enemy);  //use when enemy in your range
	void AutoAttack(void);					  //main attack
	void SendEnemyInfo(void);
}  // namespace Attack

namespace Log
{
	const char kStatus[9][20] = { "RELAX", "ON_PLANE", "JUMPING", "MOVING", "SHOOTING",
		"PICKUP", "MOVING_SHOOTING", "DEAD", "REAL_DEAD" };
	const char kDrawCommand[3][50] = { "%.3f %.3f\n", "Segment %.3f %.3f %.3f %.3f\n", "Circle %.3f %.3f %.3f\n" };
	enum DrawCommand
	{
		DRAW_POINT = 0,
		DRAW_SEGMENT,
		DRAW_CIRCLE,
		DRAW_COMMAND_SZ
	};
	enum LogFile
	{
		LOG = 0,
		PATH,
		SSSP,
		POISON,
		VERTEX,
		LOG_FILE_SZ
	};

	char log_file_name[5][20];
	FILE *log_file_handle[5];
	void InitLogFiles(void), MainLog(void), PoisonLog(void), PathLog(void);
	void DrawPath(Graph::VertexID s, Graph::VertexID t);

}  // namespace Log

using Communication::GetMyState;
using Communication::Order;
using Communication::State;
using Communication::StateSend;
using Communication::team_state;
using Geometry::Distance;
using Geometry::ToXYPos;
using Graph::VertexID;

void MoveTo(XYPosition dest, double view_angle = -1.0);
double PoisonTime(XYPosition p);
double MoveToTime(XYPosition src, XYPosition dest);
double MinMoveToTime(PolarPosition p) { return MoveToTime(pos, ToXYPos(p)); }
unsigned int Random(int rand_max = 65537) { return (unsigned int)rand_dev() % rand_max; }
unsigned int PseudoRandom(int rand_max = 65537) { return (unsigned int)pseudo_rand() % rand_max; }

//	<--------------------------- Move_control --------------------------->
XYPosition move_dest = { 500.0, 500.0 };
double view_angle = -1.0;
bool nearing_item = 0;
void CasualWalk(void), FetchItem(void), EscapePoison(void);

void play_game()
{
	int begin_time = clock();

	fprintf(stderr, "%d\tframe : %d start.\n", info.player_ID, frame);
	prev_info = info, update_info();
	if (info.poison.move_flag == 0)
	{
		info.poison.current_center = info.poison.next_center = { 500.0, 500.0 };
		info.poison.current_radius = info.poison.next_radius = 1000.0;
	}
	bool moved = false;

	Attack::UpdateAttackInfo();
	Attack::DetectEnemy();

	if (frame == 0)
	{
		Log::InitLogFiles();
		pseudo_rand.seed((unsigned int)(start_pos.x * over_pos.y));
		static const int landing_area[][2] = {
			{220, 420}, {220, 520}, {320, 520}, {420, 520}, {420, 720}, {220, 720}, {720, 620} };
		int area_id = PseudoRandom(7);
#ifdef FXXK_PUBG
		area_id = Random(7);  // For Test Purpose
#endif
		XYPosition landing_point{ landing_area[area_id][0] + Random(60), landing_area[area_id][1] + Random(60) };
		Graph::ConstructGraph();
		for (int i = 0, teammates_sz = teammates.size(); i < teammates_sz; i++)
			teammate_id[i] = teammates[i];  //init id
		parachute(SIGNALMAN, landing_point);

		fprintf(stderr, "Init Finished.\n");

		return;
	}

	Log::MainLog(), Log::PoisonLog();

	if (info.self.status == ON_PLANE || info.self.status == JUMPING)
	{
		return;
	}

	Log::PathLog();

	// Time for work!
	CasualWalk();
	FetchItem();
	Attack::AutoAttack();
	EscapePoison();

	MoveTo(move_dest, view_angle);

#ifdef FXXK_PUBG
	using namespace Log;
	log_file_handle[LOG] = fopen(log_file_name[LOG], "a+");
	fprintf(log_file_handle[LOG], "\n");
	fclose(log_file_handle[LOG]);
#endif

	int end_time = clock();
	fprintf(stderr, "%d\tframe : %d end. Single Time : %d. Total time : %d\n",
		info.player_ID, frame, end_time - begin_time, end_time);

	return;
}  //	play_game()

double PoisonTime(XYPosition p)
{
	double wait_time;
	int poison_id = 0;
	const PoisonInfo &poison = info.poison;  // Just to save some code
	if (p.x < kEps || p.x > 1000.0 || p.y < kEps || p.y > 1000.0 || poison.move_flag == 0)
		return Graph::kMaxDist * 100.0;

	while (poison_id < 8 && abs(poison.next_radius - kPoisonData[1][poison_id + 1]) >= 2.0)
		++poison_id;

	if (poison.move_flag == 2)
		wait_time = poison.rest_frames;
	else
		wait_time = 0.0;

	double dist = poison.current_radius - Distance(poison.current_center, p);
	if (dist < kEps)
		return -1.0;
	else
		return wait_time + dist / kPoisonData[2][poison_id] / 1.2;
}

void CasualWalk(void)
{
	using Graph::area_grass_start;
	using Graph::grass_vertices;
	using Graph::vertex_list;

	fprintf(stderr, "%d\tCasual Walking\n", info.player_ID);

	static const int kMaxCasualTime = 20;
	static int casual_time = 20;
	static XYPosition casual_dest{ 500.0, 500.0 };
	const PoisonInfo &poison = info.poison;
	if (Distance(pos, poison.next_center) < poison.next_radius && casual_time <= 0)
		casual_time = kMaxCasualTime;

	VertexID grass_id = -1;
	if (frame >= 800)
	{
		fprintf(stderr, "frame > 800\n");
		int area_id = Geometry::GetAreaID(pos);
		for (int i = area_grass_start[area_id]; i < area_grass_start[area_id + 1] && grass_id < 0; ++i)
			if (Distance(vertex_list[grass_vertices[i]], poison.next_center) < poison.next_radius)
				grass_id = i, casual_dest = vertex_list[grass_vertices[i]].ToXYPos();
		if (grass_id < 0)
		{
			for (int k = 0; k < 8; ++k)
			{
				int area_x = area_id % 10 + Graph::kNeighbor[k][0],
					area_y = area_id / 10 + Graph::kNeighbor[k][1];
				if (area_x < 0 || area_y < 0 || area_x > 9 || area_y > 9)
					continue;
				int neighbor_area_id = area_y * 10 + area_x;
				for (int i = area_grass_start[neighbor_area_id];
					i < area_grass_start[neighbor_area_id + 1] && grass_id < 0; ++i)
				{
					if (Distance(vertex_list[grass_vertices[i]], poison.next_center) < poison.next_radius)
						grass_id = i, casual_dest = vertex_list[grass_vertices[i]].ToXYPos();
				}
			}
		}
	}
	if ((frame < 800 || grass_id < 0) && casual_time == kMaxCasualTime)
	{
		fprintf(stderr, "frame < 800\n");
		int i = 10;
		do
		{
			double radius = (double)Random((int)std::min(100.0, info.poison.next_radius));
			double angle = Random(360);
			casual_dest.x = radius * cos(RAD(angle)) + pos.x;
			casual_dest.y = radius * sin(RAD(angle)) + pos.y;
			--i;
		} while (i >= 0 && MoveToTime(pos, casual_dest) > PoisonTime(casual_dest));
		if (i < 0) casual_dest = pos;
	}

	if (casual_dest.x < kEps || casual_dest.x > 1000.0 || casual_dest.y < kEps || casual_dest.y > 1000.0)
	{
		fprintf(stderr, "---------------------- MoveTo Out of Range ! ----------------\n");
		casual_dest = pos;
	}

	--casual_time, move_dest = casual_dest, view_angle = -1.0;

	fprintf(stderr, "%d\tCasual Walking Finished\n", info.player_ID);
	return;
}

void EscapePoison(void)
{
	fprintf(stderr, "%d\tEscaping\n", info.player_ID);

	using Geometry::Vector;
	if (Distance(pos, info.poison.next_center) < info.poison.next_radius / 2.5)
	{
		fprintf(stderr, "%d\tEscaping Fin. No Escape\n", info.player_ID);
		return;
	}
	Vector poison_dest(pos, info.poison.next_center);
	poison_dest *= ((poison_dest.Length() - info.poison.next_radius / 2.0) / poison_dest.Length());
	poison_dest += pos;
	if (MoveToTime(pos, move_dest) + MoveToTime(move_dest, poison_dest.ToXYPos()) + 10 > PoisonTime(pos))
		move_dest = info.poison.next_center;

	fprintf(stderr, "%d\tEscaping Fin. Escape\n", info.player_ID);
	return;
}

void FetchItem(void)
{
	fprintf(stderr, "%d\tFetching Item.\n", info.player_ID);
#ifdef FXXK_PUBG
	static int COUNTER = 0;
	static int LOCKON_ITEM_COUNTER = 0;
	static int PREV_ITEM_ID = 0;
	static int PICKUP_ITEM_COUNTER = 0;
	using namespace Log;
	log_file_handle[LOG] = fopen(log_file_name[LOG], "a+");
#endif

	static XYPosition now_item_xy_pos = { -1, -1 };
	static int now_item_id = -1;

	if (now_item_id > 0)
	{

		fprintf(stderr, "%d\t-----   1   -----\n", info.player_ID);

		if (Geometry::Distance(info.self.xy_pos, now_item_xy_pos) < PICKUP_DISTANCE)
		{
			fprintf(stderr, "%d\t-----   1-1   -----\n", info.player_ID);

			pickup(now_item_id);
			nearing_item = 0;
			now_item_id = -1;
#ifdef FXXK_PUBG
			PICKUP_ITEM_COUNTER++;
			fprintf(log_file_handle[LOG], "Pickedup Item Count: %d\n", PICKUP_ITEM_COUNTER);
#endif
		}
		else if (MoveToTime(pos, now_item_xy_pos) <= PoisonTime(now_item_xy_pos))
		{
			fprintf(stderr, "%d\t-----   1-2   -----\n", info.player_ID);
			move_dest = now_item_xy_pos;
			if (Geometry::Distance(info.self.xy_pos, now_item_xy_pos) < 2.5) {
				view_angle = Geometry::ToPolPos(now_item_xy_pos).angle;
				nearing_item = 1;
			}
		}
		else
		{
			fprintf(stderr, "%d\t-----   1-3   -----\n", info.player_ID);
			now_item_id = -1;
			nearing_item = 0;
		}

		fprintf(stderr, "%d\t-----   1---   -----\n", info.player_ID);
	}
	else if (!info.items.empty())  //see something
	{

		fprintf(stderr, "%d\t-----   2   -----\n", info.player_ID);
		Item closest_item;
		closest_item.polar_pos.distance = Graph::kMaxDist;
		for (int i = 0, items_sz = info.items.size(); i < items_sz; ++i)
		{
			if (ITEM_DATA[info.items[i].type].mode == TRIGGERED && info.self.vocation != HACK)
				continue;
			if (ITEM_DATA[info.items[i].type].mode == SPENDABLE)
				continue;
			if (ITEM_DATA[info.items[i].type].type == WEAPON && Attack::my_bullet >= 100)
				continue;

			if (info.items[i].polar_pos.distance < closest_item.polar_pos.distance)
				closest_item = info.items[i];
		}

		if (closest_item.polar_pos.distance == Graph::kMaxDist)
		{
#ifdef FXXK_PUBG
			fclose(log_file_handle[LOG]);
#endif
			return;
		}

		fprintf(stderr, "%d\t-----   2-1   -----\n", info.player_ID);

		if (closest_item.polar_pos.distance < PICKUP_DISTANCE)
		{
			pickup(closest_item.item_ID);
#ifdef FXXK_PUBG
			PICKUP_ITEM_COUNTER++;
			fprintf(log_file_handle[LOG], "Picking up item %d\nPickedup Item Count: %d\n", closest_item.type, PICKUP_ITEM_COUNTER);
#endif
		}
		else if (closest_item.polar_pos.distance < 10)
		{

			fprintf(stderr, "%d\t-----   2-2   -----\n", info.player_ID);

			move_dest = ToXYPos(closest_item.polar_pos);
			now_item_id = closest_item.item_ID;
			now_item_xy_pos = ToXYPos(closest_item.polar_pos);
#ifdef FXXK_PUBG
			COUNTER++;
			fprintf(log_file_handle[LOG], "Going to pick up item %d ,\txy: (%f, %f)\nCOUNTER: %d\n",
				closest_item.item_ID, ToXYPos(closest_item.polar_pos).x, ToXYPos(closest_item.polar_pos).y, COUNTER);
#endif
		}
		else if (MinMoveToTime(closest_item.polar_pos) <= PoisonTime(ToXYPos(closest_item.polar_pos)))
		{
			fprintf(stderr, "%d\t-----   2-3   -----\n", info.player_ID);

			move_dest = ToXYPos(closest_item.polar_pos);
			now_item_id = closest_item.item_ID;
			now_item_xy_pos = ToXYPos(closest_item.polar_pos);
#ifdef FXXK_PUBG
			COUNTER++;
			fprintf(log_file_handle[LOG], "Going to pick up item %d ,\txy: (%f, %f)\nCOUNTER: %d\n",
				closest_item.item_ID, ToXYPos(closest_item.polar_pos).x, ToXYPos(closest_item.polar_pos).y, COUNTER);
			if (closest_item.item_ID != PREV_ITEM_ID)
			{
				PREV_ITEM_ID = closest_item.item_ID;
				LOCKON_ITEM_COUNTER++;
				fprintf(log_file_handle[LOG], "Lockon Item Count: %d\n", LOCKON_ITEM_COUNTER);
			}
#endif
		}

		fprintf(stderr, "%d\t-----   2---   -----\n", info.player_ID);
	}

#ifdef FXXK_PUBG
	fclose(log_file_handle[LOG]);
#endif

	fprintf(stderr, "%d\tFetchItem Fin.\n", info.player_ID);
	return;
}

//	进行至多一次move()操作，使人物移动向@dest
//	输入参数：dest 目标点的坐标, view_angle 移动到的视角，若不设置将随机
//	函数里覆写了这个变量，不能声明为（常）引用
void MoveTo(XYPosition dest, double view_angle)
{
	if (dest.x < kEps || dest.x > 1000.0 || dest.y < kEps || dest.y > 1000.0)
	{
		fprintf(stderr, "---------------------- MoveTo Out of Range ! ----------------\n");
		dest = pos;
	}

	using Geometry::RelAngle;
	using Geometry::Vector;
	using Graph::NearestVertex;
	using Graph::vertex_list;
	using Graph::VertexID;

	//	长期和短期目标，长期是目的地附近的控制点，短期是下一个要跑向的控制点
	static VertexID short_term_dest = -1, long_term_dest = -1;

#ifdef FXXK_PUBG
	using namespace Log;
	log_file_handle[LOG] = fopen(log_file_name[LOG], "a+");
	fprintf(log_file_handle[LOG], "MoveToDest: %.3f %.3f ,\tinit view angle: %.3f\n",
		dest.x, dest.y, view_angle);
	fclose(log_file_handle[LOG]);
#endif

	if (Graph::IntersectsBlock(pos, dest))  //	目的地和当前位置能直接到达的话就不寻路了
	{
		VertexID s = NearestVertex(pos), t = NearestVertex(dest);
		if (s == -1 || s >= Graph::vertex_list_sz || t == -1 || t >= Graph::vertex_list_sz)
			fprintf(stderr, "---------- MoveTo Error ! Pos: %.3f %.3f Dest : %.3f %.3f\n",
				pos.x, pos.y, dest.x, dest.y);
		Graph::SSSP(t, s);
		if (t != long_term_dest)  //	长期目标有变化，需要重新规划路径
		{
			//	一个Trick，从目的地出发算一遍最短路，因为目的地不常变化
			//	而且生成的最短路树上的父结点正好是路径上的后继结点，不用把路径再反着折腾一遍
			long_term_dest = t, short_term_dest = s;
#ifdef FXXK_PUBG
			//	输出控制点信息
			log_file_handle[VERTEX] = fopen(log_file_name[VERTEX], "a+");

			fprintf(log_file_handle[VERTEX], "self pos: %.2f %.2f\tdest: %.2f %.2f\t",
				pos.x, pos.y, dest.x, dest.y);
			fprintf(log_file_handle[VERTEX], "s: % .2f % .2f\tt: %.2f %.2f\n",
				vertex_list[s].GetX(), vertex_list[s].GetY(),
				vertex_list[t].GetX(), vertex_list[t].GetY());
			fclose(log_file_handle[VERTEX]);
#endif
		}

		if (Distance(vertex_list[short_term_dest], pos) < 0.5)  //	距离小于0.5即视作到达
		{
			if (short_term_dest == long_term_dest)  // 到达最后一个控制点
				short_term_dest = -1;				// -1仅仅是一个标志，无实际意义
			else
				short_term_dest = Graph::path_prev[short_term_dest];  // 否则切换到下一个控制点
		}
		if (short_term_dest != -1)
			dest = Graph::vertex_list[short_term_dest].ToXYPos();
	}

	//	计算下一个目标点和当前位置的角度
	double abs_move_angle = DEG(atan2(dest.y - pos.y, dest.x - pos.x));
	if (abs_move_angle < 0.0)
		abs_move_angle += 360.0;
	if (abs(info.self.move_angle - abs_move_angle) >= 5 || info.self.move_cd == 0)
	{
		if (view_angle < 0) view_angle = 90.0;  //按照fxx建议，每次转头90度
		if (abs(pos.x - prev_pos.x) <= kEps &&
			abs(pos.y - prev_pos.y) <= kEps)  // 坐标没变就假设撞墙了，随机偏移一个角度
			abs_move_angle += Random(8) * 45.0;
		if (Distance(dest, pos) < 0.5)
		{
			move(RelAngle(abs_move_angle), view_angle, NOMOVE);
		}
		else
		{
			move(RelAngle(abs_move_angle), view_angle);
		}

#ifdef FXXK_PUBG
		log_file_handle[LOG] = fopen(log_file_name[LOG], "a+");

		fprintf(log_file_handle[LOG], "move angle: %3f ,\tmove view angle: %.3f\n",
			abs_move_angle, view_angle);
		fclose(log_file_handle[LOG]);
#endif
	}

	//fprintf(stderr, "self pos: %.2f %.2f\tdest: %.2f %.2f\t",pos.x, pos.y, dest.x, dest.y);
	//fprintf(stderr, "Moveto finished.\n");

	return;
}

//	估算移向目标点的时间，不会真正移动
//	输入参数：dest 目标点的坐标
//	函数里覆写了这个变量，不能声明为（常）引用
double MoveToTime(XYPosition src, XYPosition dest)
{
	if (dest.x < kEps || dest.x > 1000.0 || dest.y < kEps || dest.y > 1000.0) return Graph::kMaxDist * 100.0;
	if (src.x < kEps || src.x > 1000.0 || src.y < kEps || src.y > 1000.0) return Graph::kMaxDist * 100.0;

	using Geometry::RelAngle;
	using Geometry::Vector;
	using Graph::NearestVertex;
	using Graph::path_prev;
	using Graph::vertex_list;
	using Graph::VertexID;

	static const double kMoveSpeed[VOCATION_SZ] = { 1.8, 1.6, 1.2, 1.0 };
	double path_length = 0.0;
	if (Graph::IntersectsBlock(src, dest))  //	目的地和当前位置能直接到达的话就不寻路了
	{
		VertexID s = NearestVertex(src), t = NearestVertex(dest);
		//	一个Trick，从目的地出发算一遍最短路，因为目的地不常变化
		//	而且生成的最短路树上的父结点正好是路径上的后继结点，不用把路径再反着折腾一遍
		if (s == -1 || s >= Graph::vertex_list_sz || t == -1 || t >= Graph::vertex_list_sz)
			fprintf(stderr, "---------- MoveToTime Error ! Pos: %.3f %.3f Dest : %.3f %.3f\n",
				src.x, src.y, dest.x, dest.y);

		Graph::SSSP(t, s), path_length += Distance(src, vertex_list[s]) + Distance(dest, vertex_list[t]);
		while (s != t)
			path_length += Distance(vertex_list[s], vertex_list[path_prev[s]]), s = path_prev[s];
	}
	else
	{
		path_length = Distance(src, dest);
	}

	double res = path_length / (kMoveSpeed[info.self.vocation] / 3.0) * 1.5;
	return res;
}

namespace Log
{
	void InitLogFiles(void)
	{
#ifdef FXXK_PUBG
		const char kLogName[5][10] = { "log", "path", "sssp", "poison", "vertex" };
		int file_id = Random();
		for (int i = 0; i < LOG_FILE_SZ; ++i)
		{
			sprintf(log_file_name[i], "AI_%d_%s.txt", file_id, kLogName[i]);
			log_file_handle[i] = fopen(log_file_name[i], "w+");
		}
		fprintf(log_file_handle[LOG], "player_id: %d\n", info.player_ID);
		for (int i = 0; i < LOG_FILE_SZ; ++i)
			fclose(log_file_handle[i]);
#endif
		return;
	}

	void MainLog(void)
	{
#ifdef FXXK_PUBG
		log_file_handle[LOG] = fopen(log_file_name[LOG], "a+");
		fprintf(log_file_handle[LOG], "frame: %d\t\thp: %.2f\t\tposition: (%.3f, %.3f)\n",
			frame, info.self.hp, pos.x, pos.y);
		fprintf(log_file_handle[LOG], "move angle: %.3f\tview angle: %.3f\tstatus:%s\n",
			info.self.move_angle, info.self.view_angle, kStatus[info.self.status]);
		if (info.self.bag.size() > prev_info.self.bag.size())
			fprintf(stderr, "pick succeed\n");
		fclose(log_file_handle[LOG]);
#endif
		return;
	}

	//	毒圈位置变化时输出毒圈信息
	void PoisonLog(void)
	{
#ifdef FXXK_PUBG
		if (prev_info.poison.next_center.x != info.poison.next_center.x && info.poison.move_flag != 1)
		{
			log_file_handle[POISON] = fopen(log_file_name[POISON], "a+");
			fprintf(log_file_handle[POISON], kDrawCommand[DRAW_CIRCLE],
				info.poison.next_center.x, info.poison.next_center.y, info.poison.next_radius);
			fclose(log_file_handle[POISON]);
		}
#endif
		return;
	}

	void PathLog(void)
	{
#ifdef FXXK_PUBG
		log_file_handle[PATH] = fopen(log_file_name[PATH], "a+");
		fprintf(log_file_handle[PATH], kDrawCommand[DRAW_SEGMENT],
			prev_pos.x, prev_pos.y, pos.x, pos.y);
		fclose(log_file_handle[PATH]);
#endif
		return;
	}

	void DrawPath(VertexID s, VertexID t)
	{
#ifdef FXXK_PUBG
		using Geometry::Point;
		using Graph::path_prev;
		using Graph::vertex_list;
		log_file_handle[LogFile::SSSP] = fopen(log_file_name[LogFile::SSSP], "a+");
		for (; t != s && t != -1 && path_prev[t] != -1 && t != path_prev[t];
			t = path_prev[t])
		{
			const Point &u = vertex_list[t], &v = vertex_list[path_prev[t]];
			fprintf(log_file_handle[LogFile::SSSP], kDrawCommand[DRAW_SEGMENT],
				u.GetX(), u.GetY(), v.GetX(), v.GetY());
		}
		fprintf(log_file_handle[LogFile::SSSP], "\n");
		fclose(log_file_handle[LogFile::SSSP]);
#endif
		return;
	}
}  // namespace Log

//	<---------------------------- Geometry ----------------------->
namespace Geometry
{

	int Sgn(double x)
	{
		if (abs(x) < kEps)
			return 0;
		else
			return x < 0 ? -1 : 1;
	}

	//	直线距离，方便起见重载了各种参数
	double Distance(double x1, double y1, double x2, double y2)
	{
		return sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
	}

	double Distance(const XYPosition &p, const XYPosition &q)
	{
		return Distance(p.x, p.y, q.x, q.y);
	}

	double Distance(const Vector &p, const XYPosition &q)
	{
		return Distance(p.GetX(), p.GetY(), q.x, q.y);
	}

	double Distance(const XYPosition &p, const Vector &q)
	{
		return Distance(p.x, p.y, q.GetX(), q.GetY());
	}

	double Distance(const Vector &p, const Vector &q)
	{
		return Distance(p.GetX(), p.GetY(), q.GetX(), q.GetY());
	}

	XYPosition ToXYPos(const PolarPosition &p)
	{
		double x = p.distance * cos(RAD(p.angle + info.self.view_angle)) + pos.x;
		double y = p.distance * sin(RAD(p.angle + info.self.view_angle)) + pos.y;
		return XYPosition{ x, y };
	}

	PolarPosition ToPolPos(const XYPosition &p)
	{
		double distance = Distance(pos, p), angle = DEG(atan2(p.y - pos.y, p.x - pos.x));
		return PolarPosition{ distance, RelAngle(angle) };
	}

	//	相对于当前视线的角度
	double RelAngle(double angle)
	{
		angle -= info.self.view_angle;
		while (angle < 0)
			angle += 360.0;
		while (angle >= 360.0)
			angle -= 360.0;
		return angle;
	}

	typedef Vector Point;
	typedef std::pair<Point, Point> Segment;

	bool operator==(const Vector &u, const Vector &v)
	{
		return (Sgn(u.GetX() - v.GetX()) == 0) && (Sgn(u.GetY() - v.GetY()) == 0);
	}

	Vector operator+(const Vector &u, const Vector &v)
	{
		return Vector(u.GetX() + v.GetX(), u.GetY() + v.GetY());
	}

	Vector operator-(const Vector &u, const Vector &v)
	{
		return Vector(u.GetX() - v.GetX(), u.GetY() - v.GetY());
	}

	Vector operator*(double p, const Vector &u)
	{
		return Vector(u.GetX() * p, u.GetY() * p);
	}

	Vector operator*(const Vector &u, double p)
	{
		return Vector(u.GetX() * p, u.GetY() * p);
	}

	Vector operator/(const Vector &u, double p)
	{
		return Vector(u.GetX() / p, u.GetY() / p);
	}

	double DotProd(const Vector &a, const Vector &b)
	{
		return a.GetX() * b.GetX() + a.GetY() * b.GetY();
	}

	double VectorAngle(const Vector &a, const Vector &b)
	{
		return acos(DotProd(a, b) / a.Length() / b.Length());
	}

	Vector Normal(const Vector &a)
	{
		double l = a.Length();
		return Vector(-a.GetY() / l, a.GetX() / l);
	}

	//	二维叉积
	//	两个向量的叉积是一个标量，a×b的几何意义为他们所形成的平行四边形的有向面积
	//	坐标表示a = (x1, y1), b = (x2, y2), a×b = x1y2 - x2y1
	//	直观理解，假如b在a的左边，则有向面积为正，假如在右边则为负。假如b,
	//	a共线，则叉积为0.
	double CrossProd(const Vector &a, const Vector &b)
	{
		return a.GetX() * b.GetY() - a.GetY() * b.GetX();
	}

	//	点到直线的距离
	//	利用叉积求面积，然后除以平行四边形的底边长，得到平行四边形的高即点到直线的距离
	double DistToLine(const Point &p, const Point &a, const Point &b)
	{
		Vector v(p - a), u(b - a);
		return abs(CrossProd(v, u)) / u.Length();
	}

	//	点到线段的距离
	//	比点到直线的距离稍微复杂。因为是线段，所以如果平行四边形的高在区域之外的话就不合理
	//	这时候需要计算点到距离较近的端点的距离
	double DistToSegment(const Point &p, const Point &a, const Point &b)
	{
		if (a == b) return Distance(p, a);
		Vector v1(b - a), v2(p - a), v3(p - b);
		if (Sgn(DotProd(v1, v2)) < 0)
			return v2.Length();
		else if (Sgn(DotProd(v1, v3)) > 0)
			return v3.Length();
		return abs(CrossProd(v1, v2)) / v1.Length();
	}

	double DistToSegment(const Point &p, const Segment &l)
	{
		return DistToSegment(p, l.first, l.second);
	}

	//	判断两个线段是否相交
	//	跨立实验：判断一条线段的两端是否在另一条线段的两侧（两个端点与另一线段的叉积乘积为负）。
	//	需要正反判断两侧。
	bool SegmentIntersects(const Point &a, const Point &b, const Point &c, const Point &d)
	{
		double c1 = CrossProd(b - a, c - a), c2 = CrossProd(b - a, d - a);
		double d1 = CrossProd(d - c, a - c), d2 = CrossProd(d - c, b - c);
		return (Sgn(c1) * Sgn(c2) < 0 && Sgn(d1) * Sgn(d2) < 0);
	}

	bool SegmentIntersects(const Segment &a, const Segment &b)
	{
		return SegmentIntersects(a.first, a.second, b.first, b.second);
	}

	Point LineIntersection(const Point &a, const Point &a0, const Point &b, const Point &b0)
	{
		double a1 = a.GetY() - a0.GetY(), b1 = a0.GetX() - a.GetX();
		double a2 = b.GetY() - b0.GetY(), b2 = b0.GetX() - b.GetX();
		double c1 = CrossProd(a, a0), c2 = CrossProd(b, b0);
		double d = a1 * b2 - a2 * b1;
		return Point((b1 * c2 - b2 * c1) / d, (c1 * a2 - c2 * a1) / d);
	}

	//	返回该位置所属的区域编号，与MAP中的编号对应
	int GetAreaID(double x, double y)
	{
		return (int)(floor(y / 100.0) * 10 + floor(x / 100.0));
	}

	int GetAreaID(const XYPosition &p) { return GetAreaID(p.x, p.y); }

	int GetAreaID(const Point &p) { return GetAreaID(p.GetX(), p.GetY()); }

}  // namespace Geometry

//	<---------------------------- Graph --------------------------->
namespace Graph
{

	bool InsideBlock(const Point &p, const block &b)
	{
		int area_x = (int)floor((double)(p.GetX() / 100.0));
		int area_y = (int)floor((double)(p.GetY() / 100.0));
		if (b.shape == CIRCLE)
			return Distance(Point(b.x0 + area_x, b.y0 + area_y), p) < b.r + 0.5;
		else
		{
			int intersect_count = 0;
			Point q(p.GetX(), 1000);
			Point block_vertex[] = { Point(b.x0 - 0.5, b.y0 + 0.5),
				Point(b.x1 + 0.5, b.y0 + 0.5), Point(b.x1 + 0.5, b.y1 - 0.5),
				Point(b.x0 - 0.5, b.y1 - 0.5) };
			for (int i = 0; i < 3; ++i)
				intersect_count += SegmentIntersects(
					p, q, block_vertex[i], block_vertex[i + 1]);
			intersect_count +=
				SegmentIntersects(p, q, block_vertex[0], block_vertex[3]);
			return intersect_count & 1;  //  Intersect odd times
		}
	}

	bool InsideAnyBlock(const Point &p)
	{
		const vector<block> &area = AREA_DATA[MAP[Geometry::GetAreaID(p)]];
		int area_sz = area.size();
		for (int i = 0; i < area_sz; ++i)
		{
			if (area[i].type == CIRCLE_BUILDING || area[i].type == TREE ||
				(area[i].shape == RECTANGLE && area[i].type != SHALLOW_WATER))
				if (InsideBlock(p, area[i])) return true;
		}
		return false;
	}

	// 若pq所连线段与障碍物相交，返回true
	bool IntersectsBlock(const Point &p, const Point &q)
	{
		Segment seg(p, q);

		for (int i = 0; i < MAP_SZ; ++i)
		{
			const vector<block> &area = AREA_DATA[MAP[i]];
			int area_x = i % 10, area_y = i / 10, block_num = area.size();

			//	剪枝，一个区域如果离线段足够远就可以不用管
			if (DistToSegment(Point(area_x * 100 + 50, area_y * 100 + 50), seg) > 80)
				continue;

			if (MAP[i] == FOREST)  // 森林区域全是圆形的树，用点到线段距离判断
			{
				for (int j = 0; j < block_num; ++j)
				{
					const block &b = area[j];
					if (DistToSegment(Point(b.x0 + area_x * 100, b.y0 + area_y * 100), seg) <= b.r + 0.5)
						return true;
				}
			}
			else  // 剩下的用线段相交判断
			{
				for (int j = area_edges_start[i]; j < area_edges_start[i + 1]; ++j)
					if (SegmentIntersects(seg, hard_edges[j])) return true;
			}
		}
		return false;
	}

	//	找出最近的可直达的检查点
	VertexID NearestVertex(XYPosition pos)
	{
		int area_id = Geometry::GetAreaID(pos);
		VertexID nearest_vertex = -1;
		double min_dist = kMaxDist, dist = kMaxDist;
		Point p(pos);
		if (InsideAnyBlock(p))
		{
			for (VertexID i = 0; i < vertex_list_sz; ++i)
			{
				dist = Distance(vertex_list[i], pos);
				if (dist < min_dist) min_dist = dist, nearest_vertex = i;
			}
		}
		else
		{
			// 在本区域内找
			for (VertexID i = area_point_start[area_id];
				i < area_point_start[area_id + 1]; ++i)
			{
				dist = Distance(vertex_list[i], pos);
				if (dist < min_dist && !IntersectsBlock(p, vertex_list[i]))
					nearest_vertex = i, min_dist = dist;
			}
			if (nearest_vertex == -1)  // 没找到，扩大搜索范围到邻近的八个区域，时间效率可能较低
			{
				for (int k = 0; k < 8; ++k)
				{
					int area_x = area_id % 10 + kNeighbor[k][0],
						area_y = area_id / 10 + kNeighbor[k][1];
					if (area_x < 0 || area_y < 0 || area_x > 9 || area_y > 9)
						continue;
					int neighbor_area_id = area_y * 10 + area_x;
					for (VertexID i = area_point_start[neighbor_area_id];
						i < area_point_start[neighbor_area_id + 1]; ++i)
					{
						dist = Distance(vertex_list[i], pos);
						if (dist < min_dist && !IntersectsBlock(p, vertex_list[i]))
							nearest_vertex = i, min_dist = dist;
					}
				}
			}
			if (nearest_vertex == -1)
				for (VertexID i = 0; i < vertex_list_sz; ++i)
				{
					dist = Distance(vertex_list[i], pos);
					if (dist < min_dist) min_dist = dist, nearest_vertex = i;
				}
		}
		return nearest_vertex;
	}

	void AddBiEdge(VertexID u, VertexID v, double w)
	{
		edge_list[++edge_num] = Edge{ v, edge_list_head[u], w };
		edge_list_head[u] = edge_num;
		edge_list[++edge_num] = Edge{ u, edge_list_head[v], w };
		edge_list_head[v] = edge_num;
	}

	void ConstructVertexList(void)
	{
		vertex_list.clear(), hard_edges.clear(), grass_vertices.clear();

		static const vector<Point> kCityBlackList = {
			Point(4.5, 6.0), Point(6.5, 5.5), Point(94.0, 4.5), Point(94.5, 6.5),
			Point(6.0, 95.5), Point(5.5, 93.5), Point(95.5, 94.0), Point(93.5, 94.5) };
		static const vector<Point> kWhiteList[AREA_SZ] = {
			{},
			{Point(30, 50), Point(55, 70)},
			{Point(15, 90), Point(30, 70), Point(85, 90), Point(55, 30),
				Point(85, 10)},
			{},
			{Point(50, 5), Point(50, 95), Point(5, 50), Point(95, 50), Point(20, 50),
				Point(50, 50), Point(80, 50), Point(6.5, 6.5), Point(6.5, 93.5),
				Point(93.5, 6.5), Point(93.5, 93.5), Point(4.5, 20), Point(20, 4.5),
				Point(80, 4.5), Point(95.5, 20), Point(4.5, 80), Point(20, 95.5),
				Point(80, 95.5), Point(95.5, 80)},
			{Point(20, 90), Point(40, 90), Point(50, 90), Point(70, 90),
				Point(20, 10), Point(40, 10), Point(50, 10), Point(70, 10)},
			{Point(20, 20), Point(40, 40), Point(60, 60), Point(80, 80)},
			{Point(20, 80), Point(40, 60), Point(60, 40), Point(80, 20)} };
		static const vector<Point> kForestBlackList = {
			Point(45, 5), Point(75, 5), Point(45, 15), Point(75, 15),
			Point(5, 25), Point(95, 25), Point(25, 35), Point(45, 35),
			Point(95, 35), Point(65, 55), Point(55, 65), Point(15, 85),
			Point(65, 85), Point(65, 95), Point(85, 95) };

		static const int kCityBlackList_sz = kCityBlackList.size();
		static const int kForestBlackList_sz = kForestBlackList.size();

		for (int i = 0; i < MAP_SZ; ++i)
		{
			const vector<block> &area = AREA_DATA[MAP[i]];
			int area_x = i % 10, area_y = i / 10, block_num = area.size();
			area_point_start[i] = vertex_list.size();
			area_edges_start[i] = hard_edges.size();
			area_grass_start[i] = grass_vertices.size();

			for (int j = 0, kWhiteList_sz = kWhiteList[MAP[i]].size();
				j < kWhiteList_sz; ++j)
				vertex_list.push_back(kWhiteList[MAP[i]][j] +
					Point(area_x * 100, area_y * 100));

			if (MAP[i] == FOREST)
			{
				for (int i = 0; i < 10; ++i)
					for (int j = 0, l; j < 10; ++j)
					{
						for (l = 0; l < kForestBlackList_sz; ++l)
							if (Distance(kForestBlackList[l],
								XYPosition{ i * 10 + 5.0, j * 10 + 5.0 }) < 0.6)
								break;
						if (l == kForestBlackList_sz)
							vertex_list.push_back(
								Point(i * 10 + 5 + area_x * 100, j * 10 + 5 + area_y * 100));
					}
			}
			else
			{
				for (int j = 0; j < block_num; ++j)
				{
					block b = area[j];
					b.x0 += area_x * 100, b.x1 += area_x * 100;
					b.y0 += area_y * 100, b.y1 += area_y * 100;

					if (b.type == RECTANGLE_GRASS)
					{
						for (int k = 1; k < 4; ++k)
						{
							grass_vertices.push_back(vertex_list.size());
							vertex_list.push_back(
								Point((b.x0 + b.x1) / 2.0, b.y1 + (b.y0 - b.y1) * k / 4.0));
						}
					}
					else if (b.type == CIRCLE_GRASS)
					{
						grass_vertices.push_back(vertex_list.size());
						vertex_list.push_back(Point(b.x0, b.y0));
					}
					else if (b.shape == RECTANGLE && b.type != SHALLOW_WATER)
					{
						// NW, NE, SE, SW
						Point point[] = {
							Point(b.x0 - 0.5, b.y0 + 0.5), Point(b.x1 + 0.5, b.y0 + 0.5),
							Point(b.x1 + 0.5, b.y1 - 0.5), Point(b.x0 - 0.5, b.y1 - 0.5) };
						if (b.type == WALL)
						{
							for (int k = 0, l; k < 4; ++k)
							{
								for (l = 0; l < kCityBlackList_sz; ++l)
									if ((point[k] - Vector(area_x * 100, area_y * 100) -
										kCityBlackList[l])
										.Length() < 0.6)
										break;
								if (l == kCityBlackList_sz) vertex_list.push_back(point[k]);
							}
						}
						else if (b.type != DEEP_WATER)
						{
							for (int k = 0; k < 4; ++k)
								vertex_list.push_back(point[k]);
						}

						for (int k = 0; k < 4; ++k)
							for (int l = k + 1; l < 4; ++l)
								hard_edges.push_back(Segment(point[k], point[l]));
					}
					else if (b.shape == CIRCLE)
					{
						// Using rectangle. NW, NE, SE, SW
						b.x0 -= b.r, b.y0 += b.r;
						b.x1 = b.x0 + 2 * b.r, b.y1 = b.y0 - 2 * b.r;
						Point point[] = {
							Point(b.x0 - 0.5, b.y0 + 0.5), Point(b.x1 + 0.5, b.y0 + 0.5),
							Point(b.x1 + 0.5, b.y1 - 0.5), Point(b.x0 - 0.5, b.y1 - 0.5) };

						for (int k = 0; k < 4; ++k)
							vertex_list.push_back(point[k]);

						for (int k = 0; k < 4; ++k)
							for (int l = k + 1; l < 4; ++l)
								hard_edges.push_back(Segment(point[k], point[l]));
					}
				}
			}
		}

		vertex_list_sz = area_point_start[MAP_SZ] = vertex_list.size();
		hard_edges_sz = area_edges_start[MAP_SZ] = hard_edges.size();
		grass_vertices_sz = area_grass_start[MAP_SZ] = grass_vertices.size();

		return;
	}

	void ConstructGraph(void)
	{
		ConstructVertexList();
		//  Then construct edge list
		memset(edge_list_head, 0, sizeof(edge_list_head));
		using std::string;
		string coded_edges(coded_edges1);
		coded_edges += coded_edges2, coded_edges += coded_edges3, coded_edges += coded_edges4;
		int decode[128] = { 0 }, coded_edges_sz = coded_edges.length();
		for (int i = 0; i < 10; ++i)
			decode[i + '0'] = i;
		for (int i = 0; i < 26; ++i)
			decode[i + 'a'] = i + 10, decode[i + 'A'] = i + 10 + 26;
		decode['!'] = 62, decode['@'] = 63, decode['#'] = 64, decode['$'] = 65;
		decode['%'] = 66, decode['^'] = 67, decode['&'] = 68, decode['*'] = 69;
		for (int i = 0; i < coded_edges_sz; i += 4)
		{
			int u = decode[coded_edges[i]] * kEdgeBase + decode[coded_edges[i + 1]],
				v = decode[coded_edges[i + 2]] * kEdgeBase + decode[coded_edges[i + 3]];
			AddBiEdge(u, v, Distance(vertex_list[u], vertex_list[v]));
		}
		fprintf(stderr, "vertex_list_sz: %d, edge_num: %d\n", vertex_list_sz, edge_num);

		return;
	}

	//  Dijkstra algorithm
	void SSSP(VertexID s, VertexID t)
	{

		if (s == -1 || t == -1) fprintf(stderr, "------------- SSSP Error! ----------------\n");
		static VertexID prev_s = -1;
		if (s == prev_s)
			return;
		else
			prev_s = s;

		for (int i = 0; i < vertex_list_sz; ++i)
			dist[i] = kMaxDist, path_prev[i] = i;
		while (!sssp_heap.empty())
			sssp_heap.pop();
		path_prev[s] = s, dist[s] = 0.0, sssp_heap.push(Pdi(dist[s], s));

		while (!sssp_heap.empty())
		{
			int u = sssp_heap.top().second;
			double dist_u = sssp_heap.top().first;
			sssp_heap.pop();
			for (int i = edge_list_head[u]; i; i = edge_list[i].next_)
			{
				int v = edge_list[i].v_;
				double w = edge_list[i].w_;
				if (dist_u + w < dist[v])
					dist[v] = dist_u + w, sssp_heap.push(Pdi(dist[v], v)), path_prev[v] = u;
			}
		}
		Log::DrawPath(s, t);
		return;
	}

}  // namespace Graph

namespace Communication
{
	Order OrderDecode(int msg)  //refresh order
	{
		Order order;
		order.y = (double)(msg & 0x03FF);
		order.x = (double)((msg & 0x000FFC00) >> 10);
		order.id = (msg & 0x00F00000) >> 20;
		order.pick = msg & 0x01000000;
		order.attack = msg & 0x02000000;
		order.avoid = msg & 0x04000000;
		return order;
	}

	int OrderEncode(const Order &order)  //encode a message
	{
		int msg = 0;
		msg |= 0x10000000;
		msg |= (int)order.y;
		msg |= (int)order.x << 10;
		msg |= order.id << 20;
		msg |= order.pick << 24;
		msg |= order.attack << 25;
		msg |= order.avoid << 26;
		return msg;
	}

	State StateDecode(int msg)
	{
		State state;
		state.y = (double)(msg & 0x03FF);
		state.x = (double)((msg & 0x000FFC00) >> 10);
		state.gun = msg & 0x00100000;
		state.medicine = msg & 0x00200000;
		state.save = msg & 0x00400000;
		return state;
	}

	int StateEncode(const State &state)
	{
		int msg = 0;
		msg |= (int)state.y;
		msg |= (int)state.x << 10;
		msg |= state.gun << 20;
		msg |= state.medicine << 21;
		msg |= state.save << 22;
		msg |= state.sender_vocation << 23;
		return msg;
	}

	void OrderSend(const Order &order, int target)  
	{
		int msg = OrderEncode(order);
		if (target != info.player_ID)
			radio(target, msg);
		else
			return;
	}

	State GetMyState(void)
	{
		State state;
		state.sender_vocation = info.self.vocation;
		state.x = info.self.xy_pos.x;
		state.y = info.self.xy_pos.y;
		//以下待修改
		state.gun = (Attack::my_bullet < 30);
		state.medicine = false;
		state.save = (info.self.hp == 0);
		return state;
	}

	void StateSend(const State &state, VOCATION target)  //默认群发
	{
		int msg = StateEncode(state);
		for (int i = 0; i < 4; i++)
		{
			if (((i == target) || (target == VOCATION_SZ)) && (i != info.self.vocation))
				radio(teammate_id[i], msg);
		}
	}

	void StateReceive(const Sound &radio)
	{
		State temp = StateDecode(radio.parameter);
		VOCATION sender = (VOCATION)temp.sender_vocation;
		team_state[sender] = temp;
	}
}  // namespace Communication

//	<---------------------------- Attack --------------------------->
namespace Attack
{
	//update farthest_range,my_bullet,determine farthest weapon,then determine attack mode
	void UpdateAttackInfo(void)
	{
		medcine[0] = 0;  //bandage
		medcine[1] = 0;  //first aid
		farthest_range = 1;
		my_bullet = 0;
		my_weapon.clear();
		for (int i = 0, bag_sz = info.self.bag.size(); i < bag_sz; i++)
		{
			if (ITEM_DATA[info.self.bag[i].type].type == WEAPON && info.self.bag[i].durability > 0)
			{
				if ((info.self.bag[i].type != FIST) && (info.self.bag[i].type != TIGER_BILLOW_HAMMER))
					my_bullet += info.self.bag[i].durability;
				if (ITEM_DATA[info.self.bag[i].type].range > farthest_range)
				{
					farthest_range = ITEM_DATA[info.self.bag[i].type].range;
				}
				my_weapon.push_back(info.self.bag[i].type);
			}
			if ((info.self.bag[i].type == BONDAGE) && info.self.bag[i].durability > 0)
				medcine[0] = info.self.bag[i].durability;
			else if ((info.self.bag[i].type == FIRST_AID_CASE) && info.self.bag[i].durability > 0)
				medcine[1] = info.self.bag[i].durability;
		}
		if ((info.self.hp / VOCATION_DATA[info.self.vocation].hp >= 0.5) && (my_bullet >= 20))
			sparta_mode = true;
	}

	void SmartShoot(const OtherInfo &enemy)  //use when enemy in your range
	{
		double distance = enemy.polar_pos.distance;
		ITEM weapon = FIST;
		for (int i = 0, weapon_sz = my_weapon.size(); i < weapon_sz; i++)
		{
			double temp = ITEM_DATA[my_weapon[i]].range - distance;
			if (temp >= 0 && weapon_dps[my_weapon[i]] > weapon_dps[weapon])
				weapon = my_weapon[i];  //choose weapon: within distance, largest dps
		}
		if (enemy.status == MOVING || enemy.status == MOVING_SHOOTING)
		{
			XYPosition enemy_xy = ToXYPos(enemy.polar_pos);
			double enemy_speed = enemy.move_speed / 3;
			enemy_xy.x += enemy_speed * cos(enemy.move_angle);
			enemy_xy.y += enemy_speed * sin(enemy.move_angle);
			shoot(weapon, Geometry::ToPolPos(enemy_xy).angle);
		}
		else
			shoot(weapon, enemy.polar_pos.angle);
	}

	void DetectEnemy(void)
	{
		enemy_amount = 0;
		if (!info.others.empty())
		{
			// See someone
			double others_weight[64] = { 0 };
			const int vocation_weight[4] = { 10, 7, 5, 3 };  //imporatnce of vocation
														   //MEDIC,	SIGNALMAN,	HACK,	SNIPER
			//check teammate, get enemy amount and closest enemy
			for (int i = 0, other_sz = info.others.size(); i < other_sz; ++i)
			{
				bool is_friend = false;
				for (int teammate = 0, teammates_sz = teammates.size(); teammate < teammates_sz; ++teammate)
				{
					if (info.others[i].player_ID == teammates[teammate])
					{
						is_friend = true;
						break;
					}
				}
				if ((!is_friend) && (info.others[i].status != DEAD) && (info.others[i].status != REAL_DEAD))
				{
					enemy_amount++;
					others_weight[i] = VOCATION_DATA[info.others[i].vocation].distance / info.others[i].polar_pos.distance * vocation_weight[info.others[i].vocation];  //weight calculate,distance based,add consideration of vocation
				}
			}
			int k = 0;
			for (int i = 0, other_sz = info.others.size(); i < other_sz; i++)
				if (others_weight[i] > others_weight[k]) k = i;
			attack_enemy = info.others[k];
		}
	}

	void SendEnemyInfo(void)
	{
		static int mate = 0;
		Order enemy_info;
		enemy_info.x = ToXYPos(attack_enemy.polar_pos).x;
		enemy_info.y = ToXYPos(attack_enemy.polar_pos).y;
		enemy_info.attack = true;
		if (teammates[mate] == info.player_ID)
		{
			mate++;
			if (mate >= teammates.size())mate = 0;
		}
		Communication::OrderSend(enemy_info, teammates[mate]);
	}

	void AutoAttack(void)
	{
		bool attacking = false;  //attacking or obversing enemy
		if (enemy_amount > 0)
		{
			void SendEnemyInfo();
			if (sparta_mode)  //sparta the enemy
			{
				if (attack_enemy.polar_pos.distance <= farthest_range)
				{
					attacking = true;
					//just attack
					if (info.self.attack_cd == 0)
					{
						SmartShoot(attack_enemy);
					}
					view_angle = attack_enemy.polar_pos.angle;  //stare at him
				}
				else
				{
					move_dest = ToXYPos(attack_enemy.polar_pos);  //go to enemy position
					if(!nearing_item)
						view_angle = attack_enemy.polar_pos.angle;	//stare at him
				}
			}
			else
			{
				if (((enemy_amount > 3) && (my_bullet < 20)) ||
					(attack_enemy.polar_pos.distance > farthest_range) ||
					my_bullet <= 3)								//just flee
					if (!nearing_item)view_angle = attack_enemy.polar_pos.angle;  //stare at him
				else
				{
					attacking = true;
					if (info.self.attack_cd == 0)
					{
						SmartShoot(attack_enemy);
					}
					view_angle = attack_enemy.polar_pos.angle;  //stare at him
				}
			}
		}
		if (!attacking)  //not shooting,can help or cure
		{
			bool helping = false;
			if (info.sounds.size())//hear something
				for (int i = 0; i < info.sounds.size(); i++)
					if ((info.sounds[i].type == RADIO_VOICE) && (info.sounds[i].parameter & 0x10000000))//order
					{
						Order order = Communication::OrderDecode(info.sounds[i].parameter);
						if (!Graph::IntersectsBlock(info.self.xy_pos, { order.x,order.y }))
						{
							if (!nearing_item)
								view_angle = Geometry::ToPolPos({ order.x,order.y }).angle;
							helping = true;
							break;
						}
					}

			if (!helping)
			{
			//cure yourself
				if ((info.self.hp != VOCATION_DATA[info.self.vocation].hp) && (info.self.attack_cd == 0))
				{
					if ((VOCATION_DATA[info.self.vocation].hp - info.self.hp < 50) &&
						(VOCATION_DATA[info.self.vocation].hp - info.self.hp > 15) && (medcine[0] > 0))
						shoot(BONDAGE, 0, info.player_ID);
					else if ((VOCATION_DATA[info.self.vocation].hp - info.self.hp > 50))
						if (medcine[1])
							shoot(FIRST_AID_CASE, 0, info.player_ID);
						else if (medcine[0])
							shoot(BONDAGE, 0, info.player_ID);
				}
			}
		}
	}
}  // namespace Attack
