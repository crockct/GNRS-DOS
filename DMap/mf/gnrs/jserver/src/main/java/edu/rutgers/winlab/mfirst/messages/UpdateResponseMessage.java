package edu.rutgers.winlab.mfirst.messages;

/**
 * Created by wontoniii on 4/24/15.
 */
public class UpdateResponseMessage extends AbstractResponseMessage {
    /**
     * Creates a new instance of the message.
     */
    public UpdateResponseMessage() {
        super();
        super.type = MessageType.INSERT_RESPONSE;
    }

    @Override
    public String toString(){
        final StringBuilder sBuff = new StringBuilder("UPR #");
        sBuff.append(this.getRequestId()).append("/").append(super.responseCode);
        return sBuff.toString();
    }


    @Override
    protected int getResponsePayloadLength() {
        // No additional length
        return 0;
    }
}