package edu.rutgers.winlab.mfirst.messages;

import edu.rutgers.winlab.mfirst.GUID;
import edu.rutgers.winlab.mfirst.net.NetworkAddress;

/**
 * Created by wontoniii on 4/24/15.
 */
public class UpdateMessage extends AbstractMessage {
    /**
     * The GUID to update.
     */
    private GUID guid;

    /**
     * The set of GUID&rarr;NetworkAddress bindings for this update message.
     */
    private NetworkAddress[] bindings;

    /**
     * Creates a new update message.
     */
    public UpdateMessage() {
        super();
        super.type = MessageType.UPDATE;
    }

    /**
     * Returns the GUID for this message.
     *
     * @return the GUID for this message.
     */
    public GUID getGuid() {
        return this.guid;
    }

    /**
     * Sets the GUID for this message.
     *
     * @param guid
     *          the new GUID for this message.
     */
    public void setGuid(final GUID guid) {
        this.guid = guid;
    }

    /**
     * Gets the set of GUID&rarr;Network Address bindings for this message.
     *
     * @return the Network Address bindings for this message.
     */
    public NetworkAddress[] getBindings() {
        return this.bindings;
    }

    /**
     * Returns the number of bindings in this message.
     *
     * @return the number of bindings.
     */
    public long getNumBindings() {
        return this.bindings == null ? 0 : this.bindings.length;
    }

    /**
     * Sets the GUID&rarr;Network Address bindings for this message.
     *
     * @param bindings
     *          the new Network Address bindings for this message.
     */
    public void setBindings(final NetworkAddress[] bindings) {
        this.bindings = bindings;
    }

    @Override
    public String toString() {
        final StringBuilder sBuff = new StringBuilder("UPT #");
        sBuff.append(this.getRequestId()).append(' ').append(this.guid)
                .append(" -> {");
        if (this.bindings != null) {
            for (int i = 0; i < this.bindings.length; ++i) {
                if (i > 0) {
                    sBuff.append(", ");
                }
                sBuff.append(this.bindings[i]);
            }
        }
        sBuff.append("}");
        return sBuff.toString();
    }

    @Override
    public int getPayloadLength() {
        //  num bindings -> 4, + bindings
        int length = 4 + this.getBindingsLength();
        // GUID
        if (this.guid != null && this.guid.getBinaryForm() != null) {
            length += this.guid.getBinaryForm().length;
        }
        return length;
    }

    /**
     * The length (in bytes) of the bindings contained in this message. Only to be
     * used for network encoding.
     *
     * @return the length (in bytes) of the bindings when encoded for the network
     *         protocol.
     */
    protected int getBindingsLength() {
        int length = 0;
        if (this.bindings != null) {
            for (final NetworkAddress addr : this.bindings) {
                length += (4 + addr.getLength());
            }
        }
        return length;
    }

}