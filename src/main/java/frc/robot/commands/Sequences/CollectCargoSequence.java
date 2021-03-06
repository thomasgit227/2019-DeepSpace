/*----------------------------------------------------------------------------*/
/* Copyright (c) 2018 FIRST. All Rights Reserved.                             */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in the root directory of */
/* the project.                                                               */
/*----------------------------------------------------------------------------*/

package frc.robot.commands.Sequences;

import edu.wpi.first.wpilibj.command.CommandGroup;
import edu.wpi.first.wpilibj.command.WaitCommand;
import frc.robot.Constants;
import frc.robot.commands.ChangeElevatorPosition;
import frc.robot.commands.CollectCargo;
import frc.robot.commands.ExtendIntake;
import frc.robot.commands.RetractIntake;

public class CollectCargoSequence extends CommandGroup {

  public CollectCargoSequence() {

    addSequential(new ChangeElevatorPosition(12),.5);
    addSequential(new ExtendIntake(),.25);
    
    addParallel(new ChangeElevatorPosition(Constants.intake),.5);
    addSequential(new CollectCargo());
    
    addSequential(new WaitCommand(.35));
    addSequential(new ChangeElevatorPosition(Constants.cargoShip),.5);
    addSequential(new RetractIntake(),.1);
  }
}
