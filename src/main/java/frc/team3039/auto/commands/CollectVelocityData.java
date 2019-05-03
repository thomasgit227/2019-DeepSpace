package frc.team3039.auto.commands;

import java.util.List;

import edu.wpi.first.wpilibj.Timer;
import edu.wpi.first.wpilibj.command.Command;
import frc.team3039.robot.subsystems.Drive;
import frc.team3039.utility.DriveSignal;
import frc.team3039.utility.ReflectingCSVWriter;
import frc.team3039.utility.lib.physics.DriveCharacterization;

public class CollectVelocityData extends Command {
    private static final double kMaxPower = 0.25;
    private static final double kRampRate = 0.02;
    private static final Drive mDrive = Drive.getInstance();

    private final ReflectingCSVWriter<DriveCharacterization.VelocityDataPoint> mCSVWriter;
    private final List<DriveCharacterization.VelocityDataPoint> mVelocityData;
    private final boolean mTurn;
    private final boolean mReverse;

    private boolean isFinished = false;
    private double mStartTime = 0.0;

    /**
     * @param data     reference to the list where data points should be stored
     * @param highGear use high gear or low
     * @param reverse  if true drive in reverse, if false drive normally
     * @param turn     if true turn, if false drive straight
     */

    public CollectVelocityData(List<DriveCharacterization.VelocityDataPoint> data, boolean reverse, boolean turn) {
        mVelocityData = data;
        mReverse = reverse;
        mTurn = turn;
        mCSVWriter = new ReflectingCSVWriter<>("/home/lvuser/VELOCITY_DATA.csv", DriveCharacterization.VelocityDataPoint.class);
    }

    @Override
    public void initialize() {
        mStartTime = Timer.getFPGATimestamp();
    }

    @Override
    public void execute() {
        double percentPower = kRampRate * (Timer.getFPGATimestamp() - mStartTime);
        if (percentPower > kMaxPower) {
            isFinished = true;
            return;
        }
        System.out.println("Velocity Power = " + percentPower);
        mDrive.setOpenLoop(new DriveSignal((mReverse ? -1.0 : 1.0) * percentPower, (mReverse ? -1.0 : 1.0) * (mTurn ? -1.0 : 1.0) * percentPower));
        mVelocityData.add(new DriveCharacterization.VelocityDataPoint(
                (Math.abs(mDrive.getLeftVelocityNativeUnits()) + Math.abs(mDrive.getRightVelocityNativeUnits())) / 4096.0 * Math.PI * 10, //convert velocity to radians per second
                percentPower * 12.0 //convert to volts
        ));
        mCSVWriter.add(mVelocityData.get(mVelocityData.size() - 1));
    }

    @Override
    public boolean isFinished() {
        return isFinished;
    }

    @Override
    public void end() {
        mDrive.setOpenLoop(DriveSignal.BRAKE);
        mCSVWriter.flush();
    }

    @Override
    protected void interrupted() {
  
    }
}