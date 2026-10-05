# Using Tailscale Funnel for remote access

A gadget reaches its Hermes over any network that gives it internet access. When the gadget leaves the home Wi-Fi, Tailscale Funnel on the Hermes computer lets it phone home from anywhere with no other device or tunnel app in between.

## When this matters

The gadget stores one Wi-Fi network at a time. At home, it connects to your router and reaches the Hermes computer over the local network or tailnet. Away from home, the gadget needs a network that can reach the Hermes address you entered. Two patterns work:

- **Tailscale Funnel (recommended):** the Hermes computer publishes its gadget port on the public internet through Tailscale. The gadget connects from any network, including a phone's personal hotspot, without joining your tailnet.
- **A reachable address on the local network:** for example, a port forward. This works but exposes more and moves with your router.

## One-time setup on the Hermes computer

Tailscale must be installed and signed in to your tailnet (https://tailscale.com). Then:

1. Find the gadget plugin's port:

   ```bash
   hermes gadget info
   ```

   The device URL shown here contains the port (8765 by default when configured for remote access).

2. Publish that port through Funnel:

   ```bash
   tailscale funnel --bg <port>
   ```

   Tailscale prints the public funnel URL, for example `https://your-hostname.tail1234.ts.net`. Test with the same URL in a browser on a phone using mobile data.

3. Keep Funnel enabled. The funnel survives reboots while Tailscale runs. To unpublish: `tailscale funnel reset`.

Note: Tailscale Funnel terminates TLS with a Let's Encrypt certificate for your tailnet DNS name. The gadget connects over `wss://` (secure WebSocket) to the funnel URL, so enter the full `https://...` address from `tailscale funnel status` as the device URL, not a bare `ws://` address.

## Configuring the gadget

Enter the funnel URL as the Hermes address during any of these flows:

- The browser installer's Wi-Fi step (first flash).
- [Phone setup](setup-board.md#set-up-wi-fi-with-your-phone) (any time).
- USB console: `wifi-setup`.

The gadget has a single saved network at a time. When you leave home with a phone:

1. Enable the phone's personal hotspot (2.4 GHz Wi-Fi sharing).
2. On the gadget, open device settings and choose **Wi-Fi setup** (or power it on near home and use phone setup).
3. Join the gadget's `Hermes-XXXX` setup network from the phone, open `http://192.168.4.1`, and enter the hotspot's name and password with the funnel URL as the Hermes address.
4. Save. The gadget joins the hotspot and reaches Hermes through Funnel.

The same funnel URL works on every network, so you enter it once and never again. Saved passwords and pairing survive network changes (see the notes in [Set up Wi-Fi with your phone](setup-board.md#set-up-wi-fi-with-your-phone)).

## Security notes

- Funnel publishes exactly the ports you list, nothing else, and TLS terminates at the Tailscale process with an automatic certificate.
- The gadget authenticates to the plugin at the WebSocket layer (device pairing plus token); the funnel URL alone grants no access to other gateway features.
- Prefer a funnel URL over port forwarding on the router: no router configuration, no static exposure, and the address follows your tailnet DNS name rather than your home IP.

## Troubleshooting

- `tailscale funnel status` shows every published funnel. Verify the gadget's port is listed.
- The gadget shows a connection error but the funnel works in a browser: check that the device URL includes `https://` (wss transport) and the exact funnel hostname.
- Corporate or hotel networks that trap clients (captive portals) need the portal accepted on a laptop or phone first; the gadget cannot click through a portal itself. A phone hotspot avoids this entirely.
