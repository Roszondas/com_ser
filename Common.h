#define	TIMEOUT			3

#define COM_HNDSHAKE	"787\n"
#define COM_READY		"325\n"

enum {INTRFCE_COM, INTRFCE_TCP};
enum port_state {WAITING, HNDSHAKED, READING};
